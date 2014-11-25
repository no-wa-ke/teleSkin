#pragma once

#include "ofMain.h"

class QueuedImage {
public:
    ofPixels image;
    string filename;
	QueuedImage(ofPixels& image, string filename)
	:image(image)
	,filename(filename) {
	}
};

class ThreadedImageSaver : public ofThread {
private:
    queue< ofPtr<QueuedImage> > queue;
public:
    void threadedFunction() {
        while(!queue.empty()) {
			if(!isThreadRunning()) {
				ofLogWarning("ThreadedImageSaver") << queue.size() << " images left to save";
			}
			ofPtr<QueuedImage> cur = queue.front();
			ofSaveImage(cur->image, cur->filename, OF_IMAGE_QUALITY_HIGH);
			lock();
			queue.pop();
			unlock();
        }
    }
    void saveImage(ofPixels& img, string filename) {
		ofPtr<QueuedImage> cur(new QueuedImage(img, filename));
		lock();
		queue.push(cur);
		unlock();
		if(!isThreadRunning()){
			startThread();
		}
    }
	int getQueueSize() {
		lock();
		int size = queue.size();
		unlock();
		return size;
	}
	void exit() {
		waitForThread();
	}
};

class MultiThreadedImageSaver {
protected:
	int currentThread;
	vector< ofPtr<ThreadedImageSaver> > threads;
public:
	MultiThreadedImageSaver(int threadCount = 4)
	:currentThread(0) {
		for(int i = 0; i < threadCount; i++) {
			threads.push_back(ofPtr<ThreadedImageSaver>(new ThreadedImageSaver()));
		}
	}
	void saveImage(ofPixels& img, string filename) {
		threads[currentThread]->saveImage(img, filename);
		currentThread = (currentThread + 1) % threads.size();
	}
	int getQueueSize() {
		int size = 0;
		for(int i = 0; i < threads.size(); i++) {
			size += threads[i]->getQueueSize();
		}
		return size;
	}
	void exit() {
		for(int i = 0; i < threads.size(); i++) {
			threads[i]->waitForThread();
		}
	}
};