# cvimagepicker
Image file picker using opencv GUI win32, gtk3 and cocoa in c++17
you can use mouse (Double clicked is not available on macos) or keyboard to select an image

1 To select file  -> Use arrow keys, page up or down to browse images or folder and enter
 
2 To browse folder or image -> use key 'f' or 'i'

3 To browse parent folder -> use key Home (https://en.wikipedia.org/wiki/Home_key) or fn up arrow on cocoa


Sample 

	#include <iostream>
	#include <filesystem>

	#include <opencv2/opencv.hpp>
	#include <opencv2/highgui.hpp>

	#include "imagepicker.hpp"

	using namespace cv;
	using namespace std;


	int main(int argc, char** argv)
	{
		string imageFolder(".");
		if (!std::filesystem::is_directory(imageFolder))
		{
			cout << imageFolder << " is not a path to a folder\n";
			cout << "set . as default folder\n";
			imageFolder = ".";
		}
		ImagePicker imageSelec(imageFolder);
		Mat img;

		while (imageSelec.run())
		{
			string fileName = imageSelec.path();
			img = imread(fileName);
			imshow("Choosen image", img);
		}
	}
