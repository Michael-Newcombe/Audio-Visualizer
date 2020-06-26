#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    //initalizing sample rate and buffer size
    int sampleRate = 44100;
    int bufferSize = 512;
    //passing the sample rate and buffer size variables into the openFramesworks maxi setup function with 2 output channels
    ofxMaxiSettings::setup(sampleRate, 2, bufferSize);
    //loading the audio files
    song1.load(ofToDataPath("bensound-dubstep.wav"));
    song2.load(ofToDataPath("bensound-allthat.wav"));
    song3.load(ofToDataPath("bensound-moose.wav"));
    
    //creating a ofsoundStreamsetting object so the program can access the computers sound card and play audio with 2 output channels and 0 inputChannels
    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.sampleRate = sampleRate;
    settings.numOutputChannels = 2;
    settings.numInputChannels = 0;
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);
    
    //creating granular synthesis by pushing new maxiTimePitchStretch objects into the stretches vector where each maxiTimePitchStretch object contains a grain player for each of the maxi audio samples
    stretches.push_back(new maxiTimePitchStretch<grainPlayerWin, maxiSample>(&song1));
    stretches.push_back(new maxiTimePitchStretch<grainPlayerWin, maxiSample>(&song2));
    stretches.push_back(new maxiTimePitchStretch<grainPlayerWin, maxiSample>(&song3));
    //initializing the grain length value
    grainLength = 0.05;
    //initializing the wave value
    wave = 0.0f;
    //initializing the current sample
    currentSong = 0;
    
    //fft setup
    fftSize = 512;
    int windowSize = 512;
    int hopSize = 256;
    fft.setup(fftSize, windowSize, hopSize);
    //oct setup
    int nAverages = 12;
    oct.setup(sampleRate, fftSize/2, nAverages);
    
    //initializing spectralCentroid variable
    spectralCentroid = 0.0f;
    
    //giving the 3D primitive object an initial size
    sphere.setRadius(200);
    cone.setRadius(200);
    cone2.setRadius(200);
    //sphere resolution
    ofSetSphereResolution(24);

    
    //lighting setup
    ofSetSmoothLighting(true);
    //using a spot light
    spotLight.setSpotlight();
    //setting spot light postion
    spotLight.setPosition(ofGetWidth()*0.5, ofGetHeight()*0.01, 1000);
    
    //using a point light
    pointLight.setPointLight();
    pointLight2.setPointLight();
    
    
    //gui setup
    gui.setup("Audio Settings");
    gui.add(playbackSpeed.set("Playback Speed",1,0.1,2));
    
    //when program starts the cone primitive will display
    coneToggle = true;
    sphereToggle = false;
    octavesToggle = false;
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //initializing the spotlight value to use the octave analyzer averages value however i am passing it through a ofClamp function with a max limit of 255, as i want to use this data an RGB value
    float spotLightValue = ofClamp(*oct.averages, 0, 255);
    //setting the spotlight diffuse colour and passing in the clamped oct averages value then dividing it by 255 as ofFloatColor only accepts value from 0 to 1. I am passing the spotLightValue variable into each r,g,b value to create a white spotlight
    spotLight.setDiffuseColor(ofFloatColor(spotLightValue/255, spotLightValue/255, spotLightValue/255));
    
    //variables for r,g,b float values
    float rFloat = 0;
    float gFloat = 0;
    float bFloat = 0;
    //assigning each variable above to different octave analyzer maxAverages values using the array index of the OctaveAnalyzer peaks object. I am also clamping the values again as i want to use this data for r,g,b values.
    rFloat = ofClamp(oct.peaks[0], 0, 255);
    gFloat = ofClamp(oct.peaks[12], 0, 255);
    bFloat = ofClamp(oct.peaks[24], 0, 100);
    //passing in the maxAverages values into the pointlight diffuse color, as all three maxAverages values variables mainly return different values, passing each of these value into the r,g,b channels creates a stoplight which produces a wide range of different colours
    pointLight.setDiffuseColor(ofFloatColor(rFloat/255,gFloat/255,bFloat/255));
// cout << "oct[1] " << oct.peaks[0]<<  " oct[2] " << oct.peaks[12] << " oct[2] " << oct.peaks[24] << endl;
//  cout << "rFloat " << rFloat<<  " gFloat " << gFloat << " bFloat " << bFloat << endl;
    //calling the pointlight set postion function in update so the spotlight can move around.
    //setting the x position at half width of the screen plus the cos function which contains the returns the amount of time since the application started in seconds as a float. I am then multiplying the cos function by the width of the screen. For the y axis I am doing things except I am using the height of the screen. This allows the light to move by it's self on the x and y axis creating a sort of circular motion
    pointLight.setPosition((ofGetWidth()*0.5)+ cos(ofGetElapsedTimef()*2)*(ofGetWidth()),
                            ofGetHeight()*0.5 + cos(ofGetElapsedTimef()*0.5)*(ofGetHeight()), -200);
    //for the postion of pointLight2 I am using the same method as before but only on the y axis therefore the light moves up and down
    pointLight2.setPosition((ofGetWidth()*0.5),
                            ofGetHeight()*0.5 + cos(ofGetElapsedTimef()*0.5)*(ofGetHeight()),                            200
);
//  cout << peakFreq << endl;
    //initializing the r value for pointLight2 to the peak frequency value and passing it through a ofNormalize function so the value ranges from 0 to 1. I am passing it through a ofNormalize function instead a ofClamp because a lot of the time the value is over 255
    float pointLight2Rfloat = ofNormalize(peakFreq, 0, 10000);
    //passing in the peak frequecny value for the red channel for the diffuse color of the point2Light2, then for the green channel i am passing in the gFloat variable containing a maxAverage value, then for the blue channel i am passing in the octave analyzer averages value. As of three of theses values are different this light also produces a wide range of different colours
    pointLight2.setDiffuseColor(ofFloatColor(pointLight2Rfloat,gFloat/255,spotLightValue/255));

}

//--------------------------------------------------------------
void ofApp::draw(){

    //using cos and passing in the elasped means this variable can be used to create a continues rotation
    float spin = cos(ofGetElapsedTimef()*.00010f);
    
    //enabling the lights
    ofEnableLighting();
    spotLight.enable();
    pointLight.enable();
    pointLight2.enable();
    
    //if coneToggle is true the cone primitives are displayed
    if (coneToggle == true) {
        //rendering 4 cone primitives objects
        for (int i=0; i < 4; i++) {
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
            //roting the cones on the x axis using the spin variable
            cone.rotateDeg(spin, 0.1, 0.0, 0.0);
            //spacing out the cones slightly on the y axis
            cone.setPosition(0, i*8, 0);
            //drawing the objects wire frame instead of the faces so that I can see-through the object
            cone.drawWireframe();
            //passing the fft spectral centroid value in the setRadius function so that the cone will change size depending on the value of the spectral centroid. I am also passing this through a ofClamp function so that the size of the cone is never bigger than 300
            cone.setRadius(ofClamp(spectralCentroid*0.09, 0, 300));
            ofPopMatrix();
        }
        //rendering 4 more cone primitives objects
        for (int i=0; i < 4; i++) {
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
            //here i am putting minius infront of the spin variable so that these cones rotate in the opposite direction
            cone2.rotateDeg(-spin, 0.1, 0.0, 0.0);
            cone2.setPosition(0, i*8, 0);
            ofNoFill();
            cone2.drawWireframe();
            cone2.setRadius(ofClamp(spectralCentroid*0.09, 0, 300));
            ofPopMatrix();
        }
    }
    
    //if sphereToggle is true the sphere primitives are displayed
    if (sphereToggle == true) {
    //rendering 2 sphere primitives objects
    for (int i=0; i < 2; i++) {
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
            //spacing out the spheres on the z axis
            sphere.setPosition(0,0,i*20);
            //rotating y axis
            sphere.panDeg(spin*0.5);
            sphere.drawWireframe();
            //using the spectral centroid to change the size of the sphere
            sphere.setRadius(ofClamp(spectralCentroid*0.09f, 0, 300));
            ofPopMatrix();
        }
    }
    
    //octave analyzer drawing settings
    //width of the analyzer
    float analyzerWidth = ofGetWidth()*.50;
    //if octavesToggle is true then the octave analyzer drawing is displayed
    if (octavesToggle == true) {
        ofPushMatrix();
        ofTranslate(0, ofGetHeight()/1.75);
        //initializing the variable for incrementing the analyzer to half the screen size divided by the number of averaging bins
        float analyzerInc = analyzerWidth / oct.nAverages;
        //looping through the number of averaging bins
        for (int i = 0; i < oct.nAverages; i++) {
           //storing the octave averages values so they can be used to draw the octave analyzer, i am also mutiplying them to increase the height of the analyzer however i am also passing this into a ofClamp so that that the value is never over 300 otherwise the drawing becomes to big. By mutiplying the averages by such as large number then clamping it means this is not longer an accurate octave analyzer however doing this makes drawing look better which is what I wanted
            float height = ofClamp(oct.averages[i] * 12, 0, 300);
            //creating ofVec3f for the position of the boxes
            ofVec3f boxPos;
            //setting x,y,z postion of the boxes by setting the x position spacing using the analyzerInc variable with a offset of 250, then on the y position i am using octave averages values however i am doing 100 minus the height so that the bands are facing upwards
            boxPos.set(250 + (i * analyzerInc), 100 - height,0);
            //passing the box x,y,z coordinates into the box set position function
            box.setPosition(boxPos);
            //setting the size of the boxes with a width of 2, then for the height i am using the octave averages values
            box.set(2, height, -100);
            //instead of solid boxes i am rendering the vertices
            box.drawVertices();
            //setting the box resolution
            box.setResolution(24);
        }
        ofPopMatrix();
    }
    
    //disabling lights
    ofDisableLighting();
    //rendering gui
    gui.draw();
    
    ofDrawBitmapString("Press 1 to play song1", 12, 75);
    ofDrawBitmapString("Press 2 to play song2", 12, 100);
    ofDrawBitmapString("Press 3 to play song2", 12, 125);
    
    ofDrawBitmapString("Press c to display cones", 12, 150);
    ofDrawBitmapString("Press s to display sphere", 12, 175);
    ofDrawBitmapString("Press o to display octaves", 12, 200);

}


void ofApp::audioOut(ofSoundBuffer& output){
    //assigning the variable outChannels to the number of output channels
    std::size_t outChannels = output.getNumChannels();
    
    //looping through the output buffer
    for (int i = 0; i < output.getNumFrames(); i++){
        
        //assigning the granular synthesis sound to the variable the wave by calling the play method on the stretches vector, the value of the variable currentSong dertmines which grain player get's called, i have three grain player objects so this number ranges from 0 to 2. Now that a sample has been passed into a grain player it can be processed by a maxi fft
        wave = stretches[currentSong]->play(1, playbackSpeed, grainLength, 4, 0);
 
        //passing the sound into the fft
        fft.process(wave);
        
        //if the song is being processed by the fft the following code is executed
        if (fft.process(wave)) {
            //passing the magnitudes of the fft bins into the octave calcuate so that the octave analyzer can be used
            oct.calculate(fft.magnitudes);
            //storing the spectral centroid value
            spectralCentroid = fft.spectralCentroid();
            
            //code from the maxiGranular example for finding the peak frequency
            float binFreq = 44100.0 / fftSize;
            float sumFreqs = 0;
            float sumMags = 0;
            float maxFreq = 0;
            int maxBin = 0;
            //looping through the fft to find the bin with the highest magnitude
            for (int i = 0; i < fftSize / 2; i++) {
                sumFreqs += (binFreq * i) * fft.magnitudes[i];
                //adding up all of the ftt magnitudes
                sumMags += fft.magnitudes[i];
                //storing the bin with the highest magnitude in the fft frame
                if (fft.magnitudes[i] > maxFreq) {
                    maxFreq = fft.magnitudes[i];
                    maxBin = i;
                }
            }
            //max frequency calculation by multiplying the bin with the highest magnitude by the sample rate divided by the ftt size
            peakFreq = (float)maxBin * ( 44100/ fftSize);

        }
        
        //maxi mix with two output channels
        mix.stereo(wave, outputs, 0.5);
        
        //left channel output
        output[i * outChannels] = outputs[0] * 0.50;
        //right channel output
        output[i * outChannels + 1] = outputs[1] * 0.50;
    }
}
    
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch (key) {
            //if key 1 has been pressed currentSong = 0
        case '1':
            currentSong = 0;
            break;
        case '2':
            //if key 2 has been pressed currentSong = 1
            currentSong = 1;
            break;
        case '3':
            //if key 3 has been pressed currentSong = 2
            currentSong = 2;
            break;
            //if key c has been pressed only coneToggle is set to true
        case 'c':
            coneToggle = true;
            sphereToggle = false;
            octavesToggle = false;
            break;
            //if key s has been pressed only sphereToggle is set to true
        case 's':
            sphereToggle = true;
            coneToggle = false;
            octavesToggle = false;
            break;
            //if key o has been pressed only octavesToggle is set to true
        case 'o':
            octavesToggle = true;
            coneToggle = false;
            sphereToggle = false;
            break;
        default:
            break;
    }
  
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
