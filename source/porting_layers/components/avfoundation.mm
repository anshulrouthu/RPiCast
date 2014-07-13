
#import "avfoundation.h"
#import "interface.h"

@implementation Recorder

-(void)screenRecording:(NSURL *)destPath
{
    // Create a capture session
    mSession = [[AVCaptureSession alloc] init];
    
    // Set the session preset as you wish
    mSession.sessionPreset = AVCaptureSessionPresetHigh;
    
    // If you're on a multi-display system and you want to capture a secondary display,
    // you can call CGGetActiveDisplayList() to get the list of all active displays.
    // For this example, we just specify the main display.
    CGDirectDisplayID displayId = kCGDirectMainDisplay;
    
    // Create a ScreenInput with the display and add it to the session
    AVCaptureScreenInput *input = [[[AVCaptureScreenInput alloc] initWithDisplayID:displayId] autorelease];
    if (!input) {
        [mSession release];
        mSession = nil;
        return;
    }
    if ([mSession canAddInput:input])
        [mSession addInput:input];
    
    // Create a MovieFileOutput and add it to the session
//    mMovieFileOutput = [[[AVCaptureMovieFileOutput alloc] init] autorelease];
//    if ([mSession canAddOutput:mMovieFileOutput])
//        [mSession addOutput:mMovieFileOutput];
    
    videoOut = [[AVCaptureVideoDataOutput alloc] init];
    [videoOut setAlwaysDiscardsLateVideoFrames:YES];
    [videoOut setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar] forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    dispatch_queue_t videoCaptureQueue = dispatch_queue_create("Video Capture Queue", DISPATCH_QUEUE_SERIAL);
    [videoOut setSampleBufferDelegate:self queue:videoCaptureQueue];
    dispatch_release(videoCaptureQueue);
    if ([mSession canAddOutput:videoOut])
        [mSession addOutput:videoOut];
    videoConnection = [videoOut connectionWithMediaType:AVMediaTypeVideo];
    [videoOut release];
    
    
    // Start running the session
    [mSession startRunning];
    
    // Delete any existing movie file first
    if ([[NSFileManager defaultManager] fileExistsAtPath:[destPath path]])
    {
        NSError *err;
        if (![[NSFileManager defaultManager] removeItemAtPath:[destPath path] error:&err])
        {
            NSLog(@"Error deleting existing movie %@",[err localizedDescription]);
        }
    }
    
    // Start recording to the destination movie file
    // The destination path is assumed to end with ".mov", for example, @"/users/master/desktop/capture.mov"
    // Set the recording delegate to self
    [mMovieFileOutput startRecordingToOutputFileURL:destPath recordingDelegate:self];
}

-(void)finishRecord
{
    // Stop running the session
    [mSession stopRunning];
    
    // Release the session
    [mSession release];
    mSession = nil;
}

// AVCaptureFileOutputRecordingDelegate methods

- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error
{
    NSLog (@"Programming is fun!");
    NSLog(@"Did finish recording to %@ due to error %@", [outputFileURL description], [error description]);
    
    // Stop running the session
    [mSession stopRunning];
    
    // Release the session
    [mSession release];
    mSession = nil;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection
{
    
    NSLog (@"Programming is fun! ");
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
        NSLog (@"Programming is fun! 2");
    unsigned char *pixel = (unsigned char *)CVPixelBufferGetBaseAddress(pixelBuffer);
    NSLog (@"Programming is fun! 3 %lu %lu", CVPixelBufferGetWidthOfPlane(pixelBuffer, 0) ,CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0));
    //NSLog (@"Programming is fun! %@", pixel);

    senddata(pixel, CVPixelBufferGetWidthOfPlane(pixelBuffer, 0)*CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0));
    
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
}

@end

static Recorder *rec = [[Recorder alloc] init];

void initializeCapture()
{
    NSURL *baseURL = [NSURL URLWithString:@"file:/Users/anshulrouthu/Workspace/junk/screencap.mov"];
    
    [rec screenRecording:baseURL];
    
}

void uninitializeCapture()
{
    [rec finishRecord];
}