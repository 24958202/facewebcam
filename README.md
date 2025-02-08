# facewebcam
A facial recognition webcam c++20 program

## Installing OpenCV4 with OpenCV Contrib

    Follow these steps to install OpenCV4 with OpenCV Contrib on your system:

    ### Step 1: Install Dependencies
    Before building OpenCV, ensure you have the required dependencies installed. Run the following commands:

    **For Ubuntu/Debian:**
    ```bash
    sudo apt update
    sudo apt install -y build-essential cmake git libgtk-3-dev libavcodec-dev libavformat-dev libswscale-dev \
        libv4l-dev libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev gfortran openexr \
        libatlas-base-dev python3-dev python3-numpy libtbb2 libtbb-dev libdc1394-22-dev
    ```

    **For macOS:**
    ```bash
    brew install cmake pkg-config jpeg libpng libtiff openexr eigen tbb
    ```

    ### Step 2: Clone OpenCV and OpenCV Contrib Repositories
    Clone the OpenCV and OpenCV Contrib repositories from GitHub:
    ```bash
    git clone https://github.com/opencv/opencv.git
    git clone https://github.com/opencv/opencv_contrib.git
    ```

    ### Step 3: Create a Build Directory
    Create a build directory for OpenCV:
    ```bash
    mkdir -p opencv/build
    cd opencv/build
    ```

    ### Step 4: Configure the Build with CMake
    Run the following `cmake` command to configure the build. Make sure to specify the path to the OpenCV Contrib modules:
    ```bash
    cmake -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
      -D BUILD_EXAMPLES=ON \
      -D BUILD_SHARED_LIBS=ON \
      -D ENABLE_PRECOMPILED_HEADERS=OFF \
      -D ENABLE_NEON=ON \
      -D ENABLE_VFPV4=ON \
      -D OPENCV_GENERATE_PKGCONFIG=ON ..

    ```
    ### Step 5: Build OpenCV
    Compile OpenCV using `make`. Adjust the `-j` flag to match the number of CPU cores on your system:
    ```bash
    make -j$(nproc)  # For Linux
    make -j$(sysctl -n hw.ncpu)  # For macOS

    ```
    ******************************************
    If you have the following error:
    /home/user123/user123/libs/opencv/build/modules/imgproc/opencl_kernels_imgproc.cpp:879:1: error: unable to find string literal operator ‘operator""d41d8cd98f00b204e9800998ecf8427e’ with ‘const char [3]’, ‘long unsigned int’ arguments
  879 | ", "d41d8cd98f00b204e9800998ecf8427e", NULL};

    Open the file and change the content in line 879:

    struct cv::ocl::internal::ProgramEntry clahe_oclsrc={moduleName, "clahe",
    ", "d41d8cd98f00b204e9800998ecf8427e", NULL};

    to:

    struct cv::ocl::internal::ProgramEntry clahe_oclsrc={moduleName, "clahe", "d41d8cd98f00b204e9800998ecf8427e", NULL};

    ******************************************

    ### Step 6: Install OpenCV
    Install OpenCV to your system:
    ```bash
    sudo make install
    sudo ldconfig
    ```

    ### Step 7: Verify the Installation
    Verify that OpenCV is installed correctly by running the following command:
    ```bash
    pkg-config --modversion opencv4
    ```

    If OpenCV is installed correctly, it will output the version number (e.g., `4.x.x`).

    ### Notes:
    - Ensure that the `opencv_contrib` modules are included during the build process by specifying the `OPENCV_EXTRA_MODULES_PATH` in the `cmake` command.
    - If you encounter any issues, refer to the official OpenCV documentation: https://docs.opencv.org/

## Installing wxWidgets
https://docs.wxwidgets.org/3.2/overview_install.html



