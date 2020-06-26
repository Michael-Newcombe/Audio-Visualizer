#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"
#include "maxiGrains.h"
#include "ofxGui.h"
typedef hannWinFunctor grainPlayerWin;


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        //audio declarations
        void audioOut(ofSoundBuffer& output);
        ofSoundStream soundStream;
        ofxMaxiSample song1;
        ofxMaxiSample song2;
        ofxMaxiSample song3;
        vector<maxiTimePitchStretch<grainPlayerWin, maxiSample>*> stretches;
        double wave;
        double speed;
        double grainLength;
        int currentSong;
        double outputs[2];
        maxiMix mix;
        ofxMaxiFFT fft;
        ofxMaxiFFTOctaveAnalyzer oct;
        int fftSize;
        float peakFreq;
        float spectralCentroid;
    
        //graphics declarations
        ofSpherePrimitive sphere;
        ofConePrimitive cone;
        ofConePrimitive cone2;
        ofBoxPrimitive box;
        ofLight spotLight;
        ofLight pointLight;
        ofLight pointLight2;
        ofLight ambientLight;
    
        //gui
        ofxPanel gui;
        ofParameter<float> playbackSpeed;
    
        bool coneToggle;
        bool sphereToggle;
        bool octavesToggle;



    
    

    
};
