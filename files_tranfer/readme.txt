终端里输入make就能编译好了   然后直接运行就行；
把opencv配好

2024.8.15:
1. All the "CV_StsBadArg" were instead by "cv::Error::StsBadArg"
2. All the "CV_RGB2GRAY" were instead by "cv::COLOR_RGB2GRAY"
3. Function "drawMatches" should use "DrawMatchesFlags::" instead of integer num