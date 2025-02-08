#ifndef NEMSLIB_WEBCAM_H
#define NEMSLIB_WEBCAM_H
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cstdint>  // For uint32_t
#include <stdexcept>
#include <algorithm>
#include <ranges>
#include <wx/wx.h>
#include "public_variables.h"
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <limits.h> // For PATH_MAX
#elif defined(__APPLE__)
    #include <mach-o/dyld.h> // For _NSGetExecutablePath
    #include <limits.h>      // For PATH_MAX
#endif
class nemslib_webcam{
	public:
		nemslib_webcam();
		~nemslib_webcam();
		std::string getExecutablePath();
		std::string getExecutableDirectory();
		std::string str_replace(std::string&, std::string&, const std::string&);	
		void remove_space_for_filename(std::string&);
		void WriteBinaryOne_from_std(const std::string&, const std::string&);
		std::string ReadBinaryOne_from_std(const std::string&);
		void WritePeopleToBinary(const std::vector<pub_lib::PersonBinInfo>&, const std::string&);
		void ReadPeopleFromBinary(const std::string&, std::vector<pub_lib::PersonBinInfo>&);
		void WriteFriendsToBinary(const std::vector<pub_lib::Friend>&, const std::string&);
		void ReadFriendsFromBinary(const std::string&, std::vector<pub_lib::Friend>&);
		// Helper function: Convert cv::Mat to wxBitmap
		wxBitmap ConvertCvMatToWxBitmap(const cv::Mat&);
		cv::Mat ConvertWxBitmapToCvMat(const wxBitmap&);
};
#endif
