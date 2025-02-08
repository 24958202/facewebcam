#ifndef LIB_FACE_RECOG_H
#define LIB_FACE_RECOG_H
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <mutex>
#include <chrono>
#include <ranges>
#include <ctime>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include "public_variables.h"
#include "nemslib_webcam.h"
class lib_face_recog{
	private:
		std::mutex faceMutex;
		cv::dnn::Net faceNet; 
		cv::dnn::Net embedder; 
	public:
		enum CURRENT_FaceType{
				face_friends,
				face_strangers
		};
		CURRENT_FaceType current_face_type;
		bool model_files_exists(const std::string&);
		double cosineSimilarity(const cv::Mat&, const cv::Mat&);
		cv::Mat getFaceEmbedding(cv::dnn::Net&, const cv::Mat&);//much getFaceEmbedding before saving the cv::Mat
		void mark_on_the_person(cv::Mat&, const std::vector<cv::Rect>&, const std::string&);
		void person_exists(const cv::Mat&, const CURRENT_FaceType&, bool&, std::string&);
		void AddDateTimeOverlay(cv::Mat&);
		void get_strangers_and_display(std::vector<pub_lib::PersonBinInfo>&);
};
#endif



