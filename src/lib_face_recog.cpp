#include "lib_face_recog.h"
// Function to calculate cosine similarity between two embeddings
double lib_face_recog::cosineSimilarity(const cv::Mat& vec1, const cv::Mat& vec2) {
    double dot = vec1.dot(vec2);
    double norm1 = cv::norm(vec1);
    double norm2 = cv::norm(vec2);
    return (norm1 * norm2) > 0 ? dot / (norm1 * norm2) : 0.0;
}
bool lib_face_recog::model_files_exists(const std::string& model_file_path){
	if(model_file_path.empty()){
		return false;
	}
	return std::filesystem::exists(model_file_path);
}
// Extract face embeddings using a pre-trained model
cv::Mat lib_face_recog::getFaceEmbedding(cv::dnn::Net& embedder, const cv::Mat& face) {
    cv::Mat blob = cv::dnn::blobFromImage(face, 1.0 / 255.0, cv::Size(96, 96), cv::Scalar(0, 0, 0), true, false);
    embedder.setInput(blob);
    return embedder.forward().clone(); // Clone to ensure deep copy
}
void lib_face_recog::mark_on_the_person(cv::Mat& mframe, const std::vector<cv::Rect>& faces, const std::string& mark_content){
	if(mark_content.empty() || faces.empty()){
		return;
	}
	try{
		std::string str_copy = mark_content;
		std::string str_to_rp = "___";
		std::string str_rp = " ";
		size_t startPos = str_copy.find(str_to_rp);
		while (startPos != std::string::npos) {
			str_copy.replace(startPos, str_to_rp.length(), str_rp);
			startPos = str_copy.find(str_to_rp, startPos + str_rp.length());
		}
		for (size_t i = 0; i < faces.size(); ++i) {
			cv::rectangle(mframe, faces[i], cv::Scalar(0, 255, 0), 2);
			cv::putText(mframe, str_copy, cv::Point(faces[i].x, faces[i].y - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
		}
	}
	catch(...){}
}
void lib_face_recog::person_exists(const cv::Mat& img_input, const CURRENT_FaceType& face_type, bool& if_exists, std::string& person_name){
	if(img_input.empty()){
		return;
	}
	cv::Mat img_gray;
    cv::GaussianBlur(img_input, img_gray, cv::Size(5, 5), 0);
	cv::Ptr<cv::SIFT> detector = cv::SIFT::create(pub_lib::MAX_FEATURES);
    std::vector<cv::KeyPoint> keypoints_input;
    cv::Mat descriptors_input;
    detector->detectAndCompute(img_gray, cv::noArray(), keypoints_input, descriptors_input);
    if (descriptors_input.empty()) {
        return;
    }
	try{
		nemslib_webcam nemslib_j;
		std::string CurrDir = nemslib_j.getExecutableDirectory(); 
		std::string folder_to_check;
		if(face_type == CURRENT_FaceType::face_friends){
			folder_to_check = CurrDir + "/friends/faces";
		}
		else if(face_type == CURRENT_FaceType::face_strangers){
			folder_to_check = CurrDir + "/strangers/faces";
		}
		if_exists = false;
		for (const auto& entry : std::filesystem::directory_iterator(folder_to_check)) {
			if (entry.is_regular_file()) {
				cv::Mat existing_face = cv::imread(entry.path().string());
				if (existing_face.empty()) continue;
				std::string str_file_name = entry.path().stem().string();
				cv::Mat existing_face_gray;
				cv::cvtColor(existing_face, existing_face_gray, cv::COLOR_BGR2GRAY);
				std::vector<cv::KeyPoint> keypoints_existing;
				cv::Mat descriptors_existing;
				detector->detectAndCompute(existing_face_gray, cv::noArray(), keypoints_existing, descriptors_existing);
				if (descriptors_existing.empty()) {
					std::cerr << "Existing face has no descriptors." << std::endl;
					continue;
				}
				cv::BFMatcher matcher(cv::NORM_L2);
				std::vector<std::vector<cv::DMatch>> knnMatches;
				matcher.knnMatch(descriptors_input, descriptors_existing, knnMatches, 2);
				std::vector<cv::DMatch> goodMatches;
				for (const auto& match : knnMatches) {
					if (match.size() > 1 && match[0].distance < pub_lib::RATIO_THRESH * match[1].distance) {
						goodMatches.push_back(match[0]);
					}
				}
				if (goodMatches.size() > pub_lib::DE_THRESHOLD) {
					std::cout << "The face image already exists." << std::endl;
					if_exists = true;
					/*
					 * return person's name 
					*/
					std::string str_to_rp = "___";
					std::string str_rp = " ";
					nemslib_j.str_replace(str_file_name,str_to_rp,str_rp);
					person_name = str_file_name;
					break;
				}
			}//if
		}//for
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}
void lib_face_recog::AddDateTimeOverlay(cv::Mat& frame){
	try{
		// Get the current date and time
		time_t now = time(0);
		tm* local_time = localtime(&now);
		// Format the date and time as a string (e.g., "2025-01-24 19:52:14")
		char time_buffer[100];
		strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);
		// Define the position for the text (upper-right corner)
		int font_face = cv::FONT_HERSHEY_SIMPLEX;
		double font_scale = 1.0; // Font size
		int thickness = 2;      // Thickness of the text
		cv::Scalar green_color(0, 255, 0); // Green color in BGR format
		// Calculate the text size to adjust the position to the upper-right corner
		int baseline = 0;
		cv::Size text_size = cv::getTextSize(time_buffer, font_face, font_scale, thickness, &baseline);
		// Position the text in the upper-right corner
		int x = frame.cols - text_size.width - 10; // 10-pixel padding from the right edge
		int y = text_size.height + 10;             // 10-pixel padding from the top edge
		// Put the text on the frame
		cv::putText(frame, time_buffer, cv::Point(x, y), font_face, font_scale, green_color, thickness);
	}
	catch(...){}
}
void lib_face_recog::get_strangers_and_display(std::vector<pub_lib::PersonBinInfo>& strangers_list){
	if(!strangers_list.empty()){
		strangers_list.clear();
	}
	std::unordered_map<std::string,cv::Mat> face_img;
	std::unordered_map<std::string,cv::Mat> face_large_img;
	nemslib_webcam nemslib_j;
	std::string CurrDir = nemslib_j.getExecutableDirectory(); 
	try{
		for (const auto& entry : std::filesystem::directory_iterator(CurrDir + "/strangers/faces")) {
				if (entry.is_regular_file()) {
					cv::Mat existing_face = cv::imread(entry.path().string());
					if (existing_face.empty()) continue;
					std::string str_file_name = entry.path().stem().string();
					face_img[str_file_name] = existing_face;
				}
		}
		for (const auto& entry : std::filesystem::directory_iterator(CurrDir + "/strangers/faces_large")) {
				if (entry.is_regular_file()) {
					cv::Mat existing_face = cv::imread(entry.path().string());
					if (existing_face.empty()) continue;
					std::string str_file_name = entry.path().stem().string();
					face_large_img[str_file_name] = existing_face;
				}
		}
		if(!face_img.empty()){
			for(const auto& item : face_img){
				pub_lib::PersonBinInfo stranger;
				stranger.face_img = item.second;
				stranger.face_large_img = face_large_img[item.first];
				stranger.name = item.first;;
				stranger.captured_date_time = "";
				strangers_list.push_back(stranger);
			}
		}
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}