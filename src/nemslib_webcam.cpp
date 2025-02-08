#include "nemslib_webcam.h"
nemslib_webcam::nemslib_webcam(){}
nemslib_webcam::~nemslib_webcam(){}
std::string nemslib_webcam::getExecutablePath() {
    #if defined(_WIN32) || defined(_WIN64)
        // On Windows: Use GetModuleFileName
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
        if (length == 0 || length == sizeof(buffer)) {
            throw std::runtime_error("Failed to retrieve executable path on Windows!");
        }
        return std::string(buffer);
    #elif defined(__linux__)
        // On Linux: Use readlink to get the executable path
        char buffer[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (count == -1) {
            throw std::runtime_error("Failed to retrieve executable path on Linux!");
        }
        buffer[count] = '\0'; // Null-terminate the result
        return std::string(buffer);
    #elif defined(__APPLE__)
        // On macOS: Use _NSGetExecutablePath to get path
        uint32_t size = PATH_MAX;
        char buffer[PATH_MAX];
        // Dynamically resize the buffer if PATH_MAX is too small
        int result = _NSGetExecutablePath(buffer, &size);
        if (result == -1) {
            // If buffer is too small, dynamically allocate the correct size
            char* dynamic_buffer = new char[size];
            if (_NSGetExecutablePath(dynamic_buffer, &size) != 0) {
                delete[] dynamic_buffer;
                throw std::runtime_error("Failed to retrieve executable path on macOS!");
            }
            std::string executablePath(dynamic_buffer);
            delete[] dynamic_buffer;
            return executablePath;
        }
        return std::string(buffer);
    #else
        throw std::runtime_error("Unsupported platform!");
    #endif
}
std::string nemslib_webcam::getExecutableDirectory() {
    return std::filesystem::path(getExecutablePath()).parent_path().string();
}
std::string nemslib_webcam::str_replace(std::string& originalString, std::string& string_to_replace, const std::string& replacement){
	if(string_to_replace.empty()){
		return "";
	}
    size_t startPos = originalString.find(string_to_replace);
    while (startPos != std::string::npos) {
        originalString.replace(startPos, string_to_replace.length(), replacement);
        startPos = originalString.find(string_to_replace, startPos + replacement.length());
    }
    return originalString;
}
void nemslib_webcam::remove_space_for_filename(std::string& in_str){
	if(in_str.empty()){
		return;
	}
	//in_str.erase(std::remove_if(in_str.begin(), in_str.end(), [](unsigned char c) { return std::isspace(c); }), in_str.end());
//	for(char& c : in_str){
//		if(c == ' '){
//			c = '_';
//		}
//	}
	for(size_t i = 0; i < in_str.size(); ++i){
		if(in_str[i] == ' '){
			in_str.replace(i, 1 , "___");
			i += 2;
		}
	}
}
void nemslib_webcam::WriteBinaryOne_from_std(const std::string& data, const std::string& output_file_path) {
    // Check for empty input
    if (data.empty() || output_file_path.empty()) {
        std::cerr << "Error: Input or output file path cannot be empty!" << std::endl;
        return;
    }
    // Open the output file for binary writing
    std::ofstream outfile(output_file_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open output file for writing: " << output_file_path << std::endl;
        return;
    }
    // Write the size of the string first
    uint32_t string_size = static_cast<uint32_t>(data.size());
    outfile.write(reinterpret_cast<const char*>(&string_size), sizeof(uint32_t));
    if (!outfile) {
        std::cerr << "Error: Failed to write string size to binary file." << std::endl;
        return;
    }
    // Write the string data itself
    outfile.write(data.c_str(), string_size);
    if (!outfile) {
        std::cerr << "Error: Failed to write string data to binary file." << std::endl;
        return;
    }
    outfile.flush();
    outfile.close();
    std::cout << "Binary file successfully created at: " << output_file_path << std::endl;
}
std::string nemslib_webcam::ReadBinaryOne_from_std(const std::string& output_file_path) {
    if (output_file_path.empty()) {
        std::cerr << "Error: Output file path cannot be empty!" << std::endl;
        return "";
    }
    // Open the binary file for reading
    std::ifstream infile(output_file_path, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open input file for reading: " << output_file_path << std::endl;
        return "";
    }
    // Read the size of the string
    uint32_t string_size;
    infile.read(reinterpret_cast<char*>(&string_size), sizeof(uint32_t));
    if (!infile) {
        std::cerr << "Error: Failed to read string size from binary file." << std::endl;
        return "";
    }
    // Read the string data
    std::string data(string_size, '\0'); // Allocate a string of the correct size
    infile.read(&data[0], string_size);
    if (!infile) {
        std::cerr << "Error: Failed to read string data from binary file." << std::endl;
        return "";
    }
    infile.close();
    std::cout << "Binary file successfully read from: " << output_file_path << std::endl;
    return data;
}
	// Function to write a list of people to a binary file
void nemslib_webcam::WritePeopleToBinary(const std::vector<pub_lib::PersonBinInfo>& people_info, const std::string& output_file_path) {
	try{
		std::ofstream out_file(output_file_path, std::ios::binary);
		if (!out_file.is_open()) {
			throw std::runtime_error("Failed to open file for writing: " + output_file_path);
		}
		// Write the number of people
		size_t people_count = people_info.size();
		out_file.write(reinterpret_cast<const char*>(&people_count), sizeof(people_count));
		for (const auto& person : people_info) {
			// Serialize the name
			size_t name_size = person.name.size();
			out_file.write(reinterpret_cast<const char*>(&name_size), sizeof(name_size));
			out_file.write(person.name.data(), name_size);
			// Serialize the captured date and time
			size_t date_time_size = person.captured_date_time.size();
			out_file.write(reinterpret_cast<const char*>(&date_time_size), sizeof(date_time_size));
			out_file.write(person.captured_date_time.data(), date_time_size);
			// Serialize the small face image (face_img)
			int face_img_type = person.face_img.type();
			int face_img_rows = person.face_img.rows;
			int face_img_cols = person.face_img.cols;
			out_file.write(reinterpret_cast<const char*>(&face_img_type), sizeof(face_img_type));
			out_file.write(reinterpret_cast<const char*>(&face_img_rows), sizeof(face_img_rows));
			out_file.write(reinterpret_cast<const char*>(&face_img_cols), sizeof(face_img_cols));
			if (!person.face_img.empty()) {
				out_file.write(reinterpret_cast<const char*>(person.face_img.data), person.face_img.total() * person.face_img.elemSize());
			}
			// Serialize the large face image (face_large_img)
			int face_large_img_type = person.face_large_img.type();
			int face_large_img_rows = person.face_large_img.rows;
			int face_large_img_cols = person.face_large_img.cols;
			out_file.write(reinterpret_cast<const char*>(&face_large_img_type), sizeof(face_large_img_type));
			out_file.write(reinterpret_cast<const char*>(&face_large_img_rows), sizeof(face_large_img_rows));
			out_file.write(reinterpret_cast<const char*>(&face_large_img_cols), sizeof(face_large_img_cols));
			if (!person.face_large_img.empty()) {
				out_file.write(reinterpret_cast<const char*>(person.face_large_img.data), person.face_large_img.total() * person.face_large_img.elemSize());
			}
		}
		out_file.close();
	}
	catch(...){}
}
// Function to read a list of people from a binary file
void nemslib_webcam::ReadPeopleFromBinary(const std::string& input_file_path, std::vector<pub_lib::PersonBinInfo>& people_info) {
	try{
		std::ifstream in_file(input_file_path, std::ios::binary);
		if (!in_file.is_open()) {
			throw std::runtime_error("Failed to open file for reading: " + input_file_path);
		}
		// Read the number of people
		size_t people_count;
		in_file.read(reinterpret_cast<char*>(&people_count), sizeof(people_count));
		people_info.resize(people_count);
		for (auto& person : people_info) {
			// Deserialize the name
			size_t name_size;
			in_file.read(reinterpret_cast<char*>(&name_size), sizeof(name_size));
			person.name.resize(name_size);
			in_file.read(person.name.data(), name_size);
			// Deserialize the captured date and time
			size_t date_time_size;
			in_file.read(reinterpret_cast<char*>(&date_time_size), sizeof(date_time_size));
			person.captured_date_time.resize(date_time_size);
			in_file.read(person.captured_date_time.data(), date_time_size);
			// Deserialize the small face image (face_img)
			int face_img_type, face_img_rows, face_img_cols;
			in_file.read(reinterpret_cast<char*>(&face_img_type), sizeof(face_img_type));
			in_file.read(reinterpret_cast<char*>(&face_img_rows), sizeof(face_img_rows));
			in_file.read(reinterpret_cast<char*>(&face_img_cols), sizeof(face_img_cols));
			if (face_img_rows > 0 && face_img_cols > 0) {
				person.face_img.create(face_img_rows, face_img_cols, face_img_type);
				in_file.read(reinterpret_cast<char*>(person.face_img.data), person.face_img.total() * person.face_img.elemSize());
			}
			// Deserialize the large face image (face_large_img)
			int face_large_img_type, face_large_img_rows, face_large_img_cols;
			in_file.read(reinterpret_cast<char*>(&face_large_img_type), sizeof(face_large_img_type));
			in_file.read(reinterpret_cast<char*>(&face_large_img_rows), sizeof(face_large_img_rows));
			in_file.read(reinterpret_cast<char*>(&face_large_img_cols), sizeof(face_large_img_cols));
			if (face_large_img_rows > 0 && face_large_img_cols > 0) {
				person.face_large_img.create(face_large_img_rows, face_large_img_cols, face_large_img_type);
				in_file.read(reinterpret_cast<char*>(person.face_large_img.data), person.face_large_img.total() * person.face_large_img.elemSize());
			}
		}
		in_file.close();
	}
	catch(...){}
}
void nemslib_webcam::WriteFriendsToBinary(const std::vector<pub_lib::Friend>& friends, const std::string& output_file_path) {
    try {
        std::ofstream out_file(output_file_path, std::ios::binary);
        if (!out_file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + output_file_path);
        }
        // Write the number of friends
        size_t friend_count = friends.size();
        out_file.write(reinterpret_cast<const char*>(&friend_count), sizeof(friend_count));
        for (const auto& friend_obj : friends) {
            // Serialize the name
            size_t name_size = friend_obj.name.size();
            out_file.write(reinterpret_cast<const char*>(&name_size), sizeof(name_size));
            out_file.write(friend_obj.name.data(), name_size);
            // Serialize the face image (cv::Mat)
            if (friend_obj.face_image.empty()) {
                throw std::runtime_error("Invalid cv::Mat in Friend object.");
            }
            int rows = friend_obj.face_image.rows;
            int cols = friend_obj.face_image.cols;
            int type = friend_obj.face_image.type();
            bool has_alpha = (type == CV_8UC4); // Check if the image has an alpha channel
            // Write image metadata
            out_file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
            out_file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
            out_file.write(reinterpret_cast<const char*>(&type), sizeof(type));
            // Write image pixel data
            size_t data_size = friend_obj.face_image.total() * friend_obj.face_image.elemSize();
            out_file.write(reinterpret_cast<const char*>(friend_obj.face_image.data), data_size);
        }
        out_file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error in WriteFriendsToBinary: " << e.what() << std::endl;
        throw;
    }
}
void nemslib_webcam::ReadFriendsFromBinary(const std::string& input_file_path, std::vector<pub_lib::Friend>& friends) {
    try {
        std::ifstream in_file(input_file_path, std::ios::binary);
        if (!in_file.is_open()) {
            throw std::runtime_error("Failed to open file for reading: " + input_file_path);
        }
        // Read the number of friends
        size_t friend_count;
        in_file.read(reinterpret_cast<char*>(&friend_count), sizeof(friend_count));
        friends.resize(friend_count);
        for (auto& friend_obj : friends) {
            // Deserialize the name
            size_t name_size;
            in_file.read(reinterpret_cast<char*>(&name_size), sizeof(name_size));
            std::string name(name_size, '\0');
            in_file.read(&name[0], name_size);
            friend_obj.name = name;
            // Deserialize the face image (cv::Mat)
            int rows, cols, type;
            in_file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
            in_file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
            in_file.read(reinterpret_cast<char*>(&type), sizeof(type));
            // Read image pixel data
            size_t data_size = rows * cols * CV_ELEM_SIZE(type);
            std::vector<unsigned char> buffer(data_size);
            in_file.read(reinterpret_cast<char*>(buffer.data()), data_size);
            // Create cv::Mat from the buffer
            friend_obj.face_image = cv::Mat(rows, cols, type, buffer.data()).clone();
        }
        in_file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error in ReadFriendsFromBinary: " << e.what() << std::endl;
        throw;
    }
}
wxBitmap nemslib_webcam::ConvertCvMatToWxBitmap(const cv::Mat& mat){
	// Check if the mat is empty
    if (mat.empty()) {
        throw std::runtime_error("The image (cv::Mat) is empty!");
    }
    // Convert image from BGR to RGB (if needed)
    cv::Mat rgbMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    } else {
        rgbMat = mat; // Assume already single-channel or already in RGB format
    }
    // Create wxImage from cv::Mat
    wxImage wxImg(rgbMat.cols, rgbMat.rows, rgbMat.data, true); // 'true' indicates the wxImage will manage memory
    // Convert wxImage to wxBitmap
    wxBitmap wxBmp(wxImg);
    return wxBmp;
}
cv::Mat nemslib_webcam::ConvertWxBitmapToCvMat(const wxBitmap& bitmap) {
    // Check if the wxBitmap is valid
    if (!bitmap.IsOk()) {
        throw std::runtime_error("The wxBitmap is invalid!");
    }
    // Convert wxBitmap to wxImage
    wxImage wxImg = bitmap.ConvertToImage();
    // Get the dimensions of the image
    int width = wxImg.GetWidth();
    int height = wxImg.GetHeight();
    // Check if the image has an alpha channel
    bool has_alpha = wxImg.HasAlpha();
    // Create a cv::Mat to store the image data
    cv::Mat mat;
    if (has_alpha) {
        // If the image has an alpha channel, create a 4-channel (RGBA) cv::Mat
        mat = cv::Mat(height, width, CV_8UC4);
        // Copy pixel data (RGBA)
        unsigned char* mat_data = mat.data;
        unsigned char* wx_data = wxImg.GetData();
        unsigned char* alpha_data = wxImg.GetAlpha();
        for (int i = 0; i < width * height; ++i) {
            mat_data[i * 4 + 0] = wx_data[i * 3 + 0]; // R
            mat_data[i * 4 + 1] = wx_data[i * 3 + 1]; // G
            mat_data[i * 4 + 2] = wx_data[i * 3 + 2]; // B
            mat_data[i * 4 + 3] = alpha_data[i];      // A
        }
    } else {
        // If the image does not have an alpha channel, create a 3-channel (RGB) cv::Mat
        mat = cv::Mat(height, width, CV_8UC3);
        // Copy pixel data (RGB)
        unsigned char* mat_data = mat.data;
        unsigned char* wx_data = wxImg.GetData();
        for (int i = 0; i < width * height * 3; ++i) {
            mat_data[i] = wx_data[i];
        }
    }
    // Convert RGB to BGR (OpenCV uses BGR by default)
    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
    return mat;
}
