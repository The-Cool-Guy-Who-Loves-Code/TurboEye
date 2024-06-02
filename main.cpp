#include <iostream>
#include <cstdint>
#include <random>
#include <chrono>
#include <thread>
#include <cstdlib>

#include "image.h"


// Includes Platform Dependent Libaries
#ifdef _WIN32
	#include <windows.h>
#elif __unix__ 
	#include <unistd.h>
#endif


// Used To Hide The IO Cursor - Platform Dependent - Optinoal
void hideIOCursor() {
#ifdef _WIN32
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 100; // Set cursor size to 100% of character cell
	cursorInfo.bVisible = FALSE; // Hide the cursor
	SetConsoleCursorInfo(consoleHandle, &cursorInfo);
#elif __unix__
	std::cout << "\e[?25l";
#endif
}

void threadFunction(int numImages, std::string readFileDirectory, std::string writeFileDirectory, int numThreads, int threadNo, int stripWidth, int stripHeight, int width, int height, int dynamicRange) {
	// Initilise RNG
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 255);

	// The Strip Data, Usually Noise
	unsigned int* stripData = new unsigned int[stripWidth * stripHeight];

	// The Depthmap
	unsigned int* depthData = new unsigned int[width * height];

	// Used In Construction Of The Stereogram When Needing To Access Decrypted Data
	unsigned int* stereogram = new unsigned int[width * height];

	// Used To Write Encrypted Data To A File
	unsigned int* outputImage = new unsigned int[width * height];

	double numStrips = static_cast<double>(width) / stripWidth;

	for (int i = 1; i <= numImages; i++) {
		if (i % numThreads == threadNo) {
			std::string file = readFileDirectory + std::to_string(i) + ".png";
			read_png(file.c_str(), width, height, depthData);

			// Generate Noise Strips
			for (int x = 0; x < stripWidth; x++) {
				for (int y = 0; y < stripHeight; y++) {
					stripData[y * stripWidth + x] = dis(gen);
				}
			}

			// Generate RDS Based On The Strips And A Depth Map
			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					uint16_t color; //Uint16 For Contrast

					// Need One Strip To Borrow From
					if (x < stripWidth) {
						stereogram[y * width + x] = stripData[(y % stripHeight) * stripWidth + x];
						color = stripData[y * stripWidth + x];
					}
					else {

						// Access R channel of depth map, assuming the G and B channels are identical
						int depth = (depthData[y * width + x] >> 16) & 0xFF;

						// Calculate offset based on depth
						int offset = static_cast<int>((depth / 255.0) * dynamicRange);

						if (offset < 0) { offset = 0; }

						color = stereogram[y * width + (x - stripWidth + offset)];
						stereogram[y * width + x] = color;

					}

					//if (color < 128) {
					//	color /= 2;
					//}
					//else {
					//	color *= 2;
					//	if (color > 255) {
					//		color = 255;
					//	}
					//}
					// Encrypt Image Color Into An 32 bit Int (Order iS ARGB)
					outputImage[y * width + x] = (255 << 24) | (color << 16) | (color << 8) | (color << 0);
				}
			}

			// Write The RDS
			std::string exportName = writeFileDirectory + std::to_string(i) + ".png";
			write_png(exportName.c_str(), outputImage, width, height);
		}
	}

	//Delete Heap Arrays
	delete[] outputImage;
	delete[] stereogram;
	delete[] depthData;
	delete[] stripData;
}

int main()
{
	// Hide Cursor When Printing, This Is Simply A Style Choice And Is Not Required
	hideIOCursor();

	// Define Necesary Variables
	int numImages = 6572;

	unsigned int width = 480;
	unsigned int height = 360;

	unsigned int stripWidth = 100;
	unsigned int stripHeight = height;

	int numThreads = 20;

	int framerate = 30;

	int dynamicRange = 10;

	//Recalculate Based On Strip Width
	dynamicRange = (stripWidth / 100.0) * dynamicRange;
	std::cout << "Dynamic Range: " << dynamicRange << std::endl << std::endl;

	std::string readFolder = "inputs\\depth\\";
	std::string writeFolder = "outputs\\";
	std::string mp4 = "videos\\output.mp4";
	std::string audio = "bad_apple.mp3";


	auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::thread> threads;

	for (int i = 0; i < numThreads; ++i) {

		threads.emplace_back(threadFunction, numImages, readFolder, writeFolder, numThreads, i, stripWidth, stripHeight, width, height, dynamicRange);
	}

	for (auto& thread : threads) {
		thread.join();
	}

	

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	double timeTaken = duration.count();

	// Prints The Average Images Per Second
	std::cout << std::endl;
	std::cout << "Average Images Per Second: " << 1.0 / (timeTaken / numImages) << std::endl;
	std::cout << "Time Taken: " << duration << std::endl << std::endl;

	std::string command = "ffmpeg -r " + std::to_string(framerate) + " -i " + writeFolder + "%d.png "
		+ "-i " + audio +
		+" -vf \"gblur=sigma=1,eq=contrast=2.0\" " + mp4;

	if (std::system("ffmpeg -version") == 0) {
		std::cout << "ffmpeg exists converting to mp4" << std::endl;

		if (std::system(command.c_str()) != 0) {
			std::cout << "Failed To Convert To MP4" << std::endl;
		}
		else {
			std::cout << std::endl << "Converted To MP4 Sucsessfully!" << std::endl;
		}
	}
	else {
		std::cout << std::endl << "ffmpeg does not exist - cant convert to video" << std::endl;
	}

	return 0;
}