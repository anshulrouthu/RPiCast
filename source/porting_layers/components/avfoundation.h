/*
 * avfoundation.h
 *
 *  Created on: Jul 12, 2014
 *      Author: anshulrouthu
 */


#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#include <objc/objc-runtime.h>

@interface Recorder : NSObject <AVCaptureFileOutputRecordingDelegate, AVCaptureVideoDataOutputSampleBufferDelegate> {
@private
    AVCaptureSession *mSession;
    AVCaptureMovieFileOutput *mMovieFileOutput;
    AVCaptureVideoDataOutput *videoOut;
    AVCaptureConnection *videoConnection;
    NSTimer *mTimer;
}

-(void)screenRecording:(NSURL *)destPath;

@end

void initializeCapture();
void uninitializeCapture();
