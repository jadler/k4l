#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <pthread.h>
#include <libfreenect/libfreenect.h>

using namespace cv;

extern "C" {

    //    typedef void (*on_motion_cb_t)(void);
    //    on_motion_cb_t on_motion;

    typedef void (*closest_point_cb_t)(int, int);
    closest_point_cb_t closest_callback;

    struct location {
        Vec2f position;
        Vec2f speed;
        Vec2f start_pos;
        bool tracked;
        int id, size;
    };

    vector< location > prev_touches;
    vector< location > touches;
    int next_id = 1;

    uint8_t* pixel;

    Mat rawdepth(Size(640, 480), CV_16UC1);
    Mat background(Size(640, 480), CV_16UC1);
    Mat difference(Size(640, 480), CV_16UC1);

    Mat depthf(Size(640, 480), CV_8UC1);
    Mat thresh(Size(640, 480), CV_8UC1);
    Mat denoise(Size(640, 480), CV_8UC1);

    bool die;
    bool image_process = false;
    bool gesture_process = false;
    int reset_bg = 5;

    unsigned char lower_val = 8;
    unsigned char upper_val = 20;

    int min_size = 25;
    int max_size = 150;

    double tilt_angle;

    freenect_device *f_dev;
    freenect_context *f_ctx;

    pthread_t fnkt_thread;
    pthread_t imgp_thread;
    pthread_t gest_thread;
    pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;

    void set_closest_point_cb(void* closest_cb) {
        closest_callback = (closest_point_cb_t) closest_cb;
    }

    void depth_cb(freenect_device *dev, void* depth, uint32_t timestamp) {
        pthread_mutex_lock(&buf_mutex);

        //        memcpy(rawdepth.data, depth, 640 * 480 * 2);
        //
        //        pixel = (uint8_t*) rawdepth.data;
        pixel = (uint8_t*) depth;

        int closestPixel = 0;

        int _x = 0;
        int _y = 0;

        for (int y = 0; y < 480; y++) {
            for (int x = 0; x < 640; x++) {
                int i = x + y * 640;
                if (pixel[i] >= closestPixel) {
                    closestPixel = pixel[i];
                    _x = x;
                    _y = y;
                }
            }
        }
        closest_callback(_x, _y);

        pthread_mutex_unlock(&buf_mutex);
    }

    void *freenect_threadfunc(void* arg) {
        printf("freenect thread started\n");
        while (!die && freenect_process_events(f_ctx) >= 0) {
        }
        printf("freenect thread finished\n");
        return NULL;
    }

    void *image_processing_threadfunc(void* arg) {
        printf("image processing thread started\n");
        while (!die && freenect_process_events(f_ctx) >= 0) {
            // lock the buffer
            pthread_mutex_lock(&buf_mutex);

            // update background to current camera image
            if (reset_bg) {
                printf("resetting background...\n");
                rawdepth.copyTo(background);
                reset_bg--;
            }

            // subtract current image from background
            subtract(background, rawdepth, difference);

            // unlock the buffer
            pthread_mutex_unlock(&buf_mutex);
        }
        printf("image processing thread finished\n");
        return NULL;
    }

    void *gesture_recognition_threadfunc(void* arg) {
        printf("gesture processing thread started\n");
        while (!die && freenect_process_events(f_ctx) >= 0) {

        }
        printf("gesture processing thread finished\n");
        return NULL;
    }

    int setup_device() {

        die = false;

        // setup, open device etc.
        if (freenect_init(&f_ctx, NULL) < 0) {
            printf("freenect_init failed\n");
            return -1;
        }

        freenect_set_log_level(f_ctx, FREENECT_LOG_INFO);

        int nr_devices = freenect_num_devices(f_ctx);
        printf("number of devices found: %d\n", nr_devices);

        int user_device_number = 0;

        if (nr_devices < 1)
            return -1;

        if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
            printf("could not open device\n");
            return -1;
        }

        // indicate camera activity
        freenect_set_led(f_dev, LED_RED);

        // start the depth stream
        freenect_set_depth_callback(f_dev, depth_cb);
        freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED));

        return 0;
    }

    void stop_device() {
        die = true;
        printf("exiting mainloop.\n");

        freenect_set_led(f_dev, LED_BLINK_GREEN);
        freenect_set_tilt_degs(f_dev, 0);
        freenect_stop_depth(f_dev);

        freenect_close_device(f_dev);
        freenect_shutdown(f_ctx);

        pthread_join(fnkt_thread, NULL);

        if (image_process) {
            pthread_join(imgp_thread, NULL);
        }

        if (gesture_process) {
            pthread_join(gest_thread, NULL);
        }
    }

    int start_device() {

        freenect_start_depth(f_dev);

        if (pthread_create(&fnkt_thread, NULL, freenect_threadfunc, NULL)) {
            printf("pthread_create for freenect_threadfunc failed\n");
            return -1;
        }

        if (image_process) {
            if (pthread_create(&imgp_thread, NULL, image_processing_threadfunc, NULL)) {
                printf("pthread_create for image_processing_threadfunc failed\n");
                return -1;
            }
        }

        if (gesture_process) {
            if (pthread_create(&gest_thread, NULL, gesture_recognition_threadfunc, NULL)) {
                printf("pthread_create for gesture_recognition_threadfunc failed\n");
                return -1;
            }
        }

        return 0;
    }

    void reset_background() {
        reset_bg = 5;
    }

    void set_tilt_angle(double angle) {
        if (angle > 30) {
            angle = 30;
        }

        if (angle < -30) {
            angle = -30;
        }

        freenect_set_tilt_degs(f_dev, angle);
    }
}
