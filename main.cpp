#include <iostream>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "imagepicker.hpp"

using namespace cv;
using namespace std;


int main(int argc, char** argv)
{
    const String keys =
        "{Help h usage ? help  |     | Display this message   }"
        "{@arg0                | .  | Start browse from this folder}";
    CommandLineParser parser(argc, argv, keys);

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    std::string imageFolder = parser.get<string>(0);
    if (!std::filesystem::is_directory(imageFolder))
    {
        cout << imageFolder << " is not a path to a folder\n";
        cout << "set . as default folder\n";
        imageFolder = ".";
    }
    {
        ImagePicker imageSelec(imageFolder);
        imageSelec.setVerbose(true);
        Mat img;

        while (imageSelec.run())
        {
            string fileName = imageSelec.path();
            img = imread(fileName);
            imshow("Choosen image", img);
        }
    }
    int a = 0;
    cout << "Hello";
    cin >> a;

}
