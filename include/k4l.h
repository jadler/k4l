/* 
 * File:   k4l.h
 * Author: jaguar
 *
 * Created on 9 de Novembro de 2013, 19:46
 */
#ifndef K4L_H
#define	K4L_H

void set_closest_point_cb(void* closest_point_cb);

/*void set_on_motion_callback(void* on_motion_cb);

void set_on_rotation_callback(void* on_rotation_cb);

void set_on_scale_callback(void* on_scale_cb);

void get_closest_point();*/

int setup_device();

int start_device();

void stop_device();

void set_tilt_angle(double angle);

#endif	/* K4L_H */
