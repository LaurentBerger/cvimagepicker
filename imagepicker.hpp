#include <iostream>
#include <fstream>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#define KEY_CODE_QUIT 27
#define KEY_CODE_ENTER '\r'
#define KEY_CODE_PAGE_UP     -10
#define KEY_CODE_PAGE_DOWN      +10
#define KEY_CODE_ARROW_LEFT -1
#define KEY_CODE_ARROW_UP   -2
#define KEY_CODE_ARROW_DROITE +1
#define KEY_CODE_ARROW_DOWN    +2
#define KEY_CODE_HOME 5
#define KEY_CODE_MODE_IMAGE    100
#define KEY_CODE_MODE_FOLDER   -100


#define NB_LIG 5
#define NB_COL 4
#define EPAISSEUR 2
#define THUMBMAIL_SIZE 150


class ImagePicker {
public: 
    ~ImagePicker()
    {
        cv::destroyWindow(windowName_);
    }
    ImagePicker(std::string repDefaut=".", int nbLig=NB_LIG, int nbCol=NB_COL, int larImg=THUMBMAIL_SIZE, int hauImg= THUMBMAIL_SIZE) : repDefaut_(repDefaut), nbLig_(nbLig), nbCol_(nbCol),
                                                                                                                                        widthImage_(larImg), heightImage_(hauImg)
    {
        init_ = false;
        fileSelec_ = false;
        windowName_ = "Album";
    }


    bool run()
    {
        auto actIdx = std::ref(idxDir_);
        if (!init_)
            init();
        if (modeImage_)
        {
            actIdx = std::ref(idxImg_);
            actIdx.get().setSize(album_.size());
        }
        else
        {
            actIdx = std::ref(idxDir_);
            actIdx.get().setSize(folder_.size());
        }


        int keyCode = 0;
        selecSouris_ = false;
        while (keyCode != KEY_CODE_QUIT && 
               !(modeImage_ && (keyCode == KEY_CODE_ENTER || selecSouris_)))
        {
            int resVis = int(cv::getWindowProperty(windowName_, cv::WND_PROP_VISIBLE));
            if (!resVis)
            {
                if (verbose_)
                    std::cout << "Windows " << windowName_ << " closed" << std::endl;
                cv::namedWindow(windowName_);
                cv::setMouseCallback(windowName_, mouseManager, this);
            }

            updateSelection();
            cv::imshow(windowName_, fileBrowser_);
            int code = cv::waitKeyEx(20);
            if (verbose_ && code > 0)
                std::cout << "key " << std::hex << code << std::endl;
            if (platformKeyCode_.find(code) == platformKeyCode_.end())
                keyCode = 0;
            else
                keyCode = platformKeyCode_[code];
            
            if (keyCode != 0)
            {   
                step_ = 0;
                switch (keyCode) {
                case KEY_CODE_ARROW_DOWN:
                    step_ = actIdx.get().nbCol_;
                    actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                    if (actIdx.get().selec2_ >= actIdx.get().size_)
                        actIdx.get().selec2_ = actIdx.get().selec1_;
                    break;
                case KEY_CODE_ARROW_UP:
                    step_ = -actIdx.get().nbCol_;
                    actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                    if (actIdx.get().selec2_ < 0)
                        actIdx.get().selec2_ = actIdx.get().selec1_;
                    break;
                case KEY_CODE_ARROW_DROITE:
                    step_ = 1;
                    actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                    if (actIdx.get().selec2_ >= actIdx.get().size_)
                        actIdx.get().selec2_ = actIdx.get().selec1_;
                    break;
                case KEY_CODE_ARROW_LEFT:
                    step_ = -1;
                    actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                    if (actIdx.get().selec2_ < 0)
                        actIdx.get().selec2_ = actIdx.get().selec1_;
                    break;
                case KEY_CODE_PAGE_UP:
                    step_ = -actIdx.get().nbCol_ * actIdx.get().nbLig_;
                    actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                    if (actIdx.get().selec2_ < 0)
                        actIdx.get().selec2_= actIdx.get().selec1_;
                    break;
                case KEY_CODE_PAGE_DOWN:
                    step_ = actIdx.get().nbCol_ * actIdx.get().nbLig_;
                    actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                    if (actIdx.get().selec2_ >= actIdx.get().size_)
                        actIdx.get().selec2_ = actIdx.get().selec1_;
                    break;
                case KEY_CODE_MODE_IMAGE:
                    modeImage_ = true;
                    actIdx = std::ref(idxImg_);
                    drawStoreImages();
                    break;
                case KEY_CODE_MODE_FOLDER:
                    modeImage_ = false;
                    actIdx = std::ref(idxDir_);
                    drawStoreImages();
                    break;
                case KEY_CODE_HOME:
                    {
                        auto path = std::filesystem::path(repDefaut_);
                        repDefaut_ = absolute(path).parent_path().generic_string();
                        if (verbose_)
                            std::cout << "Folder " << repDefaut_ << std::endl;
                        selecSouris_ = false;
                        keyCode = 0;
                        init();
                    }
                    break;
                default:
                    break;
                }
            }
            if (!modeImage_ && (keyCode == KEY_CODE_ENTER || selecSouris_))
            {
                int idxFolder = actIdx.get().selec1_;
                repDefaut_ = actIdx.get().album_[idxFolder].second;
                if (verbose_ )
                    std::cout << "Folder " << repDefaut_ << std::endl;
                selecSouris_ = false;
                keyCode = 0;
                init();

            }
        }
        if (modeImage_)
        {
            if (keyCode == KEY_CODE_ENTER || selecSouris_)
            {
                fileName_ = album_[actIdx.get().selec1_].second;
                fileSelec_ = true;
            }
            else
            {
                fileName_ = "";
                fileSelec_ = false;
            }
        }
        else
            if (keyCode == KEY_CODE_ENTER || selecSouris_)
            {
                fileName_ = album_[actIdx.get().selec1_].second;
                fileSelec_ = false;
            }
            else
            {
                fileName_ = "";
                fileSelec_ = false;
            }

        return fileSelec_;
        
    }

    std::string path()
    {
        if (fileSelec_)
            return fileName_;
        return "";
    }

    void setVerbose(bool b)
    {
        verbose_ = b;
        if (!verbose_)
            cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
        else
            cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_INFO);
    }

protected:
    void updateSelection(void)
    {
        auto actIdx = std::ref(idxDir_);
        if (modeImage_)
        {
            actIdx = std::ref(idxImg_);
        }
        else
            actIdx = std::ref(idxDir_);
        if (actIdx.get().selec2_ < actIdx.get().first_ || actIdx.get().selec2_ >= actIdx.get().last__)
        {
            if (actIdx.get().selec2_ < actIdx.get().first_)
            {
                actIdx.get().first_ = std::max(actIdx.get().first_ + step_, 0);
                actIdx.get().last__ = std::min(int(actIdx.get().size_), actIdx.get().first_ + actIdx.get().nbCol_ * actIdx.get().nbLig_);
            }
            if (actIdx.get().selec2_ >= actIdx.get().last__)
            {
                actIdx.get().last__ = std::min(actIdx.get().last__ + step_, int(actIdx.get().size_));
                actIdx.get().first_ = std::max(0, actIdx.get().last__ - actIdx.get().nbCol_ * actIdx.get().nbLig_);
            }
            drawStoreImages();
        }
        if (actIdx.get().selec1_ != actIdx.get().selec2_)
        {
            cv::Mat dst(actIdx.get().heightImage_, actIdx.get().widthImage_, CV_8UC3, cv::Scalar::all(0));
            cv::Mat src = actIdx.get().album_[actIdx.get().selec1_].first;
            int idxImage = actIdx.get().selec1_ - actIdx.get().first_;
            if (idxImage >= 0 && idxImage < actIdx.get().nbLig_ * actIdx.get().nbCol_)
            {
                src.copyTo(dst);
                dst.copyTo(fileBrowser_(cv::Rect(actIdx.get().widthImage_ * (idxImage % actIdx.get().nbCol_), actIdx.get().heightImage_ * (idxImage / actIdx.get().nbCol_), dst.cols, dst.rows)));
            }
            actIdx.get().selec1_ = actIdx.get().selec2_;
            rectangleSelectedImage();
        }
    }
    void mouseManager(int event, int x, int y, int flags)
    {
        auto actIdx = std::ref(idxDir_);
        if (modeImage_)
        {
            actIdx = std::ref(idxImg_);
        }
        else
            actIdx = std::ref(idxDir_);
        if (x < 0 || x >= actIdx.get().widthImage_ * actIdx.get().nbCol_)
            return;
        if (y < 0 || y >= actIdx.get().heightImage_ * actIdx.get().nbLig_)
            return;
        switch (event) {
        case cv::EVENT_LBUTTONDOWN:
            actIdx.get().selec2_ = x / actIdx.get().widthImage_ + actIdx.get().nbCol_ * (y / actIdx.get().heightImage_) + actIdx.get().first_;
            if (actIdx.get().selec2_ >= actIdx.get().size_)
                actIdx.get().selec2_ = actIdx.get().selec1_;
            else
                updateSelection();
            break;
        case cv::EVENT_LBUTTONDBLCLK:
            actIdx.get().selec2_ = x / actIdx.get().widthImage_ + actIdx.get().nbCol_ * (y / actIdx.get().heightImage_) + actIdx.get().first_;
            if (actIdx.get().selec2_ >= actIdx.get().size_)
                actIdx.get().selec2_ = actIdx.get().selec1_;
            else
            {
                updateSelection();
                selecSouris_ = true;
            }
            break;
        case cv::EVENT_MOUSEWHEEL:
            if (flags > 0)
            {
                step_ = -actIdx.get().nbCol_;
                actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                if (actIdx.get().selec2_ < 0)
                    actIdx.get().selec2_ = actIdx.get().selec1_;
            }
            else
            {
                step_ = actIdx.get().nbCol_;
                actIdx.get().selec2_ = actIdx.get().selec1_ + step_;
                if (actIdx.get().selec2_ >= actIdx.get().size_)
                    actIdx.get().selec2_ = actIdx.get().selec1_;
            }
            break;
        case cv::EVENT_MOUSEMOVE:
            break;
        default:
            if (verbose_)
                std::cout << "Event " << event << "\n";

        }
    }

    static void mouseManager(int event, int x, int y, int flags, void* param)
    {
        ImagePicker* obj = (ImagePicker*)param;

        obj->mouseManager(event, x, y, flags);
    }

    void initKeyCodeMap()
    {
        std::string gui = cv::currentUIFramework();
        if (gui == "WIN32")
        {
            platformKeyCode_[0x210000] = KEY_CODE_PAGE_UP;
            platformKeyCode_[0x220000] = KEY_CODE_PAGE_DOWN;
            platformKeyCode_[0x250000] = KEY_CODE_ARROW_LEFT;
            platformKeyCode_[0x260000] = KEY_CODE_ARROW_UP;
            platformKeyCode_[0x270000] = KEY_CODE_ARROW_DROITE;
            platformKeyCode_[0x280000] = KEY_CODE_ARROW_DOWN;
            platformKeyCode_[0x240000] = KEY_CODE_HOME;
            platformKeyCode_[27] = KEY_CODE_QUIT;
            platformKeyCode_[int('\t')] = KEY_CODE_ENTER;
        }
        else if (gui == "GTK3")
        {
            platformKeyCode_[0x10ff55] = KEY_CODE_PAGE_UP;
            platformKeyCode_[0x10ff56] = KEY_CODE_PAGE_DOWN;
            platformKeyCode_[0x10ff51] = KEY_CODE_ARROW_LEFT;
            platformKeyCode_[0x10ff52] = KEY_CODE_ARROW_UP;
            platformKeyCode_[0x10ff53] = KEY_CODE_ARROW_DROITE;
            platformKeyCode_[0x10ff54] = KEY_CODE_ARROW_DOWN;
            platformKeyCode_[0x10001b] = KEY_CODE_QUIT;
            platformKeyCode_[0x10000d] = KEY_CODE_ENTER;
            platformKeyCode_[0x100066] = KEY_CODE_MODE_FOLDER;
            platformKeyCode_[0x100069] = KEY_CODE_MODE_IMAGE;
            platformKeyCode_[0x10ff50] = KEY_CODE_HOME;
        }
        else if (gui == "COCOA")
        {
            platformKeyCode_[0x10ff55] = KEY_CODE_PAGE_UP;
            platformKeyCode_[0x10ff56] = KEY_CODE_PAGE_DOWN;
            platformKeyCode_[0xf702] = KEY_CODE_ARROW_LEFT;
            platformKeyCode_[0xf700] = KEY_CODE_ARROW_UP;
            platformKeyCode_[0xf703] = KEY_CODE_ARROW_DROITE;
            platformKeyCode_[0xf701] = KEY_CODE_ARROW_DOWN;
            platformKeyCode_[0xd] = KEY_CODE_ENTER;
            platformKeyCode_[0xf72c] = KEY_CODE_HOME;
        
            platformKeyCode_[int('i')] = KEY_CODE_MODE_IMAGE;
            platformKeyCode_[0x66] = KEY_CODE_MODE_FOLDER;
        }
        else
        {
            if (verbose_)
                std::cout << gui << " Unknown HighGUI backend\n";
            platformKeyCode_[0x10001b] = KEY_CODE_QUIT;
        if (verbose_)
            std::cout << gui << " HighGUI backend\n";
        }

    }
    void rectangleSelectedImage()
    {
        auto actIdx = std::ref(idxDir_);
        if (modeImage_)
        {
            actIdx = std::ref(idxImg_);
        }
        else
            actIdx = std::ref(idxDir_);
        if (actIdx.get().album_.size() == 0)
            return;
        int idxImage = actIdx.get().selec1_ - actIdx.get().first_;
        if (idxImage < 0 || idxImage >= actIdx.get().nbCol_ * actIdx.get().nbLig_)
            return;
        cv::Mat src = actIdx.get().album_[actIdx.get().selec1_].first;
        cv::Mat dst(actIdx.get().heightImage_, actIdx.get().widthImage_, CV_8UC3, cv::Scalar::all(0));
        src.copyTo(dst);
        cv::rectangle(dst, cv::Rect(0, 0, dst.cols, dst.rows), cv::Scalar(0, 0, 255), EPAISSEUR);
        dst.copyTo(fileBrowser_(cv::Rect(actIdx.get().widthImage_ * (idxImage % actIdx.get().nbCol_), actIdx.get().heightImage_ * (idxImage / actIdx.get().nbCol_), dst.cols, dst.rows)));
    }

    void drawStoreImages()
    {
        auto actIdx = std::ref(idxDir_);
        if (modeImage_)
        {
            actIdx = std::ref(idxImg_);
        }
        else
            actIdx = std::ref(idxDir_);
        fileBrowser_.setTo(cv::Scalar::all(0));
        for (int idx = actIdx.get().first_; idx < actIdx.get().last__; idx++)
        {
            cv::Mat dst = actIdx.get().album_[idx].first;
            int idxImage = idx - actIdx.get().first_;
            dst.copyTo(fileBrowser_(cv::Rect(actIdx.get().widthImage_ * (idxImage % actIdx.get().nbCol_), actIdx.get().heightImage_ * (idxImage / actIdx.get().nbCol_), dst.cols, dst.rows)));
        }

        rectangleSelectedImage();
    }

    void initAlbum()
    {
        album_.clear();
        folder_.clear();
        idxImg_.first_ = 0;
        idxImg_.selec1_ = 0;
        idxImg_.selec2_ = 0;
        idxImg_.nbLig_ = nbLig_;
        idxImg_.nbCol_ = nbCol_;
        idxImg_.heightImage_ = heightImage_;
        idxImg_.widthImage_ = widthImage_;
        idxDir_.first_ = 0;
        idxDir_.selec1_ = 0;
        idxDir_.selec2_ = 0;
        idxDir_.heightImage_ = 40;
        idxDir_.nbLig_ = (nbLig_ * heightImage_) / idxDir_.heightImage_;
        idxDir_.nbCol_ = 1;
        idxDir_.widthImage_ = nbCol_ * widthImage_;
        fileBrowser_ = cv::Mat(nbLig_ * heightImage_, nbCol_ * widthImage_, CV_8UC3, cv::Scalar::all(0));
        const auto& entry = std::filesystem::directory_iterator(repDefaut_);
        int idxImage = 0;
        for (const auto& entry : std::filesystem::directory_iterator(repDefaut_))
        {
            auto pathStr = entry.path();
            try
            {
                cv::Mat img = cv::imread(pathStr.generic_string(), cv::IMREAD_COLOR);
                if (!img.empty())
                {
                    int lMax = std::max(img.rows, img.cols);
                    double f = double(THUMBMAIL_SIZE) / lMax;
                    cv::Mat dst;
                    cv::resize(img, dst, cv::Size(), f, f);
                    album_.push_back(std::make_pair(dst, pathStr.generic_string()));
                    idxImage++;
                    if (verbose_ && idxImage % 100 == 0)
                        std::cout << "*";
                }
                else if (std::filesystem::is_directory(pathStr))
                {
                    cv::Mat myText(idxDir_.heightImage_, idxDir_.widthImage_, CV_8UC3, cv::Scalar::all(0));
                    cv::putText(myText, pathStr.generic_string(), cv::Point(0, (3 * idxDir_.heightImage_) / 4), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 2);
                    folder_.push_back(std::make_pair(myText, pathStr.generic_string()));
                }
            }
            catch (std::filesystem::filesystem_error& e)
            {
                if (verbose_ )
                    std::cout << e.what() << "\n";

            }
        }
        if (verbose_)
        {
            std::cout << "\n Dossier " << repDefaut_ << " contenant:\n";
            std::cout << album_.size() << " images.\n";
            std::cout << folder_.size() << " dossiers.\n";
        }
        int c = 0;
        idxDir_.album_ = folder_;
        idxImg_.album_ = album_;
        idxDir_.setSize(folder_.size());
        idxImg_.setSize(album_.size());
        idxImg_.last__ = std::min(nbCol_ * nbLig_, int(album_.size()));
        idxDir_.last__ = std::min(idxDir_.nbLig_, int(folder_.size()));
        drawStoreImages();
    }

    void init()
    {
        if (!init_)
        {
            modeImage_ = true;
            cv::namedWindow(windowName_);
            cv::setMouseCallback(windowName_, mouseManager, this);
            initKeyCodeMap();
            init_ = true;
        }
        initAlbum();
    }
private:
    struct IndiceSelect {
        int selec1_;
        int selec2_;
        int first_;
        int last__;
        int nbLig_;
        int nbCol_;
        int widthImage_;
        int heightImage_;
        size_t size_;
        std::vector<std::pair<cv::Mat, std::string>> album_;
        IndiceSelect(int a=0, int b=0, int c=0, int d=0) : selec1_(a), selec2_(b), first_(c), last__(d)
        {
            size_ = 0;
        };
        size_t setSize(size_t x)
        {
            size_ = x;
            return size_;
        }

    };
    std::string repDefaut_;
    std::string fileName_;
    std::string windowName_;
    std::map<int, int> platformKeyCode_;
    bool fileSelec_;
    bool modeImage_; // true browse image, false browse folder

    int nbLig_;
    int nbCol_;
    int step_;
    int widthImage_;
    int heightImage_;
    cv::Mat fileBrowser_;
    std::vector<std::pair<cv::Mat, std::string>> album_;
    std::vector<std::pair<cv::Mat, std::string>> folder_;
    IndiceSelect idxImg_;
    IndiceSelect idxDir_;
    bool selecSouris_;
    bool init_;
    bool verbose_;
};

