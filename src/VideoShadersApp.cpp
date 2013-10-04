#include "VideoShadersApp.h"

void VideoShadersApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 700, 700 );
	settings->setFrameRate( 60.0f );
	settings->enableConsoleWindow();
}
void VideoShadersApp::setup()
{
	// Display sizes
	mMainDisplayWidth = Display::getMainDisplay()->getWidth();
	mRenderX = mMainDisplayWidth;
	mRenderY = 0;
	for (auto display : Display::getDisplays() )
	{
		//std::cout << "Reso:" << display->getHeight() << "\n"; 
		mRenderWidth = display->getWidth();
		mRenderHeight = display->getHeight();
	}
	mOriginUpperLeft = true;
	mScreenWidth = mRenderWidth;
	mScreenHeight = mRenderHeight;

	movieLoaded = false;
	isFlipped = false;
	iResolution = Vec3i( mRenderWidth, mRenderHeight, 1 );
	try 
	{
		iGlobalTime = 1;
		iMouse = Vec3i( mRenderWidth/2, mRenderHeight/2, 1 );

		/* ARB gl::Texture::Format format;
		format.setTargetRect();
		mTexture0 = gl::Texture(loadImage( loadAsset("background.jpg") ), format);*/
		mTexture0 = gl::Texture(loadImage( loadAsset("background.jpg") ) );
		//iResolution = Vec3i( mTexture0.getWidth(), mTexture0.getHeight(), 1 );
		iChannelResolution = Vec3i( mTexture0.getWidth(),  mTexture0.getHeight(), 1);
		// load and compile the shader
		mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("passthru.frag") );
		//mHotShader = GlslHotProg( "deflt.vert", "edgedetection.frag" );
	}
	catch( const std::exception &e ) 
	{
		// if anything went wrong, show it in the output window
		console() << e.what() << std::endl;
	}

	//store window
	controlWindow = this->getWindow();
	int uniqueId = getNumWindows();
	controlWindow->getSignalClose().connect(
		[uniqueId,this] { shutdown(); this->console() << "You quit console window #" << uniqueId << std::endl; }
	);

	b = 0;
	e = 0;
	vector<float> mbuffer; 
	for(int i = 0; i < 1024; i++)
	{
		mbuffer.push_back(0);        
	}
	gui = new ciUICanvas(getWindowWidth() - 300, getWindowHeight() - 200, 260, 180);
	gui->setTheme( CI_UI_THEME_RUSTICORANGE );
	speedRotary = new ciUIRotarySlider( 50, -2.0, 4.0, rate, "speed" );
	gui->addWidget( speedRotary ); 
	fps = new ciUILabel("fps", CI_UI_FONT_SMALL);
	gui->addWidgetSouthOf(fps, "speed");        
	mvg = (ciUIMovingGraph *) gui->addWidgetSouthOf(new ciUIMovingGraph(240, 30, mbuffer, 1024, 0, 120, "fpsmvg"), "fps");    

	gui->registerUIEvents(this, &VideoShadersApp::guiEvent); 
	// gStreamer
	m_dLastTime = 0;
	m_iCurrentVideo = 0;
	m_fSpeed = 1;
	m_fVolume = 1;
	m_iLoopMode = LoopMode::LOOP;
	m_iTilesDivisor = 1;
	m_fSeekPos = m_fOldSeekPos = 0;
	m_bUseVideoBuffer = true;
	m_bUseAudioBuffer = false;

	m_Font = ci::Font("Consolas", 48);
	setupGui();
	gl::enableAlphaBlending();

	receiver.setup( 10009 );
	createNewWindow();
}

void VideoShadersApp::guiEvent(ciUIEvent *event)
{
	string name = event->widget->getName(); 
	cout << name << endl; 
	if(name == "begin")
	{
		ciUIRotarySlider *begin = (ciUIRotarySlider *) event->widget; 
		b = begin->getScaledValue(); 
	}
	if(name == "end")
	{
		ciUIRotarySlider *end = (ciUIRotarySlider *) event->widget; 
		e = end->getScaledValue(); 
	}
	if(name == "speed")
	{
		ciUIRotarySlider *sp = (ciUIRotarySlider *) event->widget; 
		rate = sp->getScaledValue(); 
		//mMovie.setRate( rate );
	}	
	cout << "begin: " << b << " end: " << e  << " speed: " << rate << endl; 

	//mMovie.setActiveSegment(b, e);
}
//full screen movie
void VideoShadersApp::addFullScreenMovie( const fs::path &path )
{
	console() << "Add FullScreen Movie" << std::endl;
	try 
	{
		std::shared_ptr<GStreamerWrapper> fileToLoad = std::shared_ptr<GStreamerWrapper>(new GStreamerWrapper());
		std::string uri = "file:/" + path.string();

		if(fileToLoad->open(uri, m_bUseVideoBuffer, m_bUseAudioBuffer))
		{
			m_Players.push_back(fileToLoad);
			m_VideoTextures.push_back(gl::Texture());
			m_Players.back()->play();
		}
		
		//duration = m_Players.back()->getDurationInMs;
		/*gui->removeWidget("begin");
		gui->removeWidget("end");
		rate = 1;
		beginRotary = new ciUIRotarySlider( 50, 0.0, duration, 0.0, "begin" );
		gui->addWidgetEastOf( beginRotary, "speed" ); 
		endRotary = new ciUIRotarySlider( 50, 0.0, duration, duration, "end" );
		gui->addWidgetEastOf( endRotary, "begin" ); */

		movieLoaded = true;

		iChannelResolution = Vec3i( m_Players.back()->getWidth(), m_Players.back()->getHeight(), 1);

	}
	catch( ... ) 
	{
		console() << "Unable to load the movie." << std::endl;
	}
}
void VideoShadersApp::fileDrop( FileDropEvent event )
{ 
	// gStreamer
	for(int i=0; i<event.getFiles().size(); i++)
	{
		addFullScreenMovie( event.getFile( i ) );
		
	}
}
void VideoShadersApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		setFullScreen( ! isFullScreen() );
	}
}

// load and compile shaders
void VideoShadersApp::shaderPassThru()
{
	mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("passthru.frag") );
}
void VideoShadersApp::shaderLight()
{
	mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("light.frag") );
}
void VideoShadersApp::shaderEdgeDetection()
{
	mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("edgedetection.frag") );
}
void VideoShadersApp::shaderTest()
{
	mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("test.frag") );
}
void VideoShadersApp::shaderColors()
{
	mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("colors.frag") );
}
void VideoShadersApp::createNewWindow()
{
	WindowRef renderWindow = createWindow( Window::Format().size( mRenderWidth, mRenderHeight ) );
	// create instance of the window and store in vector
	RenderWindow rWin = RenderWindow("name", mRenderWidth, mRenderHeight, renderWindow);	
	renderWindows.push_back( rWin );
	renderWindow->setPos(mRenderX, mRenderY);
	renderWindow->setBorderless();
	renderWindow->setAlwaysOnTop();

	HWND hWnd = (HWND)renderWindow->getNative();

	HRESULT hr = S_OK;
	// Create and populate the Blur Behind structure
	DWM_BLURBEHIND bb = {0};

	// Enable Blur Behind and apply to the entire client area
	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = true;
	bb.hRgnBlur = NULL;

	// Apply Blur Behind
	hr = DwmEnableBlurBehindWindow(hWnd, &bb);
	if (SUCCEEDED(hr))
	{
		HRESULT hr = S_OK;

		// Set the margins, extending the bottom margin.
		MARGINS margins = {-1};

		// Extend the frame on the bottom of the client area.
		hr = DwmExtendFrameIntoClientArea(hWnd,&margins);
	}

	// for demonstration purposes, we'll connect a lambda unique to this window which fires on close
	int uniqueId = getNumWindows();
	renderWindow->getSignalClose().connect(
		[uniqueId,this] { shutdown(); this->console() << "You closed window #" << uniqueId << std::endl; }
	);

}
void VideoShadersApp::deleteWindows()
{
	for ( auto wRef : renderWindows ) DestroyWindow( (HWND)wRef.mWRef->getNative() );
}
void VideoShadersApp::shutdown()
{
	clearAll();
	VideoShadersApp::quit();
}
void VideoShadersApp::quitProgram()
{
	shutdown();
}
void VideoShadersApp::mouseDrag( MouseEvent event )
{
	iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
}
void VideoShadersApp::mouseMove( MouseEvent event )
{
	iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
}
void VideoShadersApp::clearAll()
{
	m_Players.clear();
	m_VideoTextures.clear();
}

void VideoShadersApp::pause()
{
	if(m_Players[m_iCurrentVideo]->getState() == PlayState::PLAYING)
		m_Players[m_iCurrentVideo]->pause();
	else
		m_Players[m_iCurrentVideo]->play();
}

void VideoShadersApp::stop()
{
	m_Players[m_iCurrentVideo]->stop();
}

void VideoShadersApp::toggleDirection()
{
	if(m_Players[m_iCurrentVideo]->getDirection() == PlayDirection::FORWARD)
		m_Players[m_iCurrentVideo]->setDirection(PlayDirection::BACKWARD);
	else
		m_Players[m_iCurrentVideo]->setDirection(PlayDirection::FORWARD);
}

void VideoShadersApp::updateGui()
{
	// Video / Audio Infos
	// update dynamic info of video/audio
	std::stringstream strTmp;
	strTmp << "label='file:  " << m_Players[m_iCurrentVideo]->getFileName() << "'";
	mParams.setOptions( "file", strTmp.str() );
	strTmp.clear();	strTmp.str("");
	strTmp << "label='frame:  " << m_Players[m_iCurrentVideo]->getCurrentFrameNumber() << "/" << m_Players[m_iCurrentVideo]->getNumberOfFrames() << "'";
	mParams.setOptions( "frame", strTmp.str() );
	strTmp.clear();	strTmp.str("");
	strTmp << "label='time:  " << m_Players[m_iCurrentVideo]->getCurrentTimeInMs() << ":" << m_Players[m_iCurrentVideo]->getDurationInMs() << "'";
	mParams.setOptions( "time", strTmp.str() );
	strTmp.clear();	strTmp.str("");
	/*strTmp << "label='video codec: " << m_Players[m_iCurrentVideo]->getVideoCodecName() << "'";
	m_Gui.setOptions("video codec", strTmp.str());*/
	strTmp.clear();	strTmp.str("");
	/*strTmp << "label='audio codec: " << m_Players[m_iCurrentVideo]->getAudioCodecName() << "'";
	m_Gui.setOptions("audio codec", strTmp.str());*/
	strTmp.clear();	strTmp.str("");
	strTmp << "label='width: " << m_Players[m_iCurrentVideo]->getWidth() << "'";
	mParams.setOptions("width", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='height: " << m_Players[m_iCurrentVideo]->getHeight() << "'";
	mParams.setOptions("height", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='fps: " << m_Players[m_iCurrentVideo]->getFps() << "'";
	mParams.setOptions("fps", strTmp.str());
	strTmp.clear();	strTmp.str("");
	//strTmp << "label='bitrate:  " << m_Players[m_iCurrentVideo]->getBitrate() << "'";
	//m_Gui.setOptions("bitrate", strTmp.str());
	strTmp.clear();	strTmp.str("");
	/*strTmp << "label='audio channels:  " << m_Players[m_iCurrentVideo]->getAudioChannels() << "'";
	m_Gui.setOptions("audio channels", strTmp.str());*/

	strTmp.clear();	strTmp.str("");
	strTmp << "label='audio sample rate:  " << m_Players[m_iCurrentVideo]->getAudioSampleRate() << "'";
	mParams.setOptions("audio sample rate", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='frame:  " << m_Players[m_iCurrentVideo]->getCurrentFrameNumber() << "/" << m_Players[m_iCurrentVideo]->getNumberOfFrames() << "'";
	mParams.setOptions("frame", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='time:  " << m_Players[m_iCurrentVideo]->getCurrentTimeInMs() << "/" << m_Players[m_iCurrentVideo]->getDurationInMs() << "'";
	mParams.setOptions("time", strTmp.str());
}

void VideoShadersApp::setupGui()
{
	std::stringstream strTmp;
	mParams = params::InterfaceGl( "Video shaders", Vec2i( 400, 700 ) );
	mParams.addParam( "Render Window X",		&mRenderX,											"" );
	mParams.addParam( "Render Window Y",		&mRenderY,											"" );
	mParams.addParam( "Render Window Width",	&mRenderWidth,										"" );
	mParams.addParam( "Render Window Height",	&mRenderHeight,										"" );
	mParams.addParam( "Flipped",				&isFlipped,											"" );
	mParams.addParam( "Screen Width",			&mScreenWidth,										"" );
	mParams.addParam( "Screen Height",			&mScreenHeight,										"" );
	mParams.addParam( "mOriginUpperLeft",		&mOriginUpperLeft,									"" );
	mParams.addButton( "Pass Through",			bind( &VideoShadersApp::shaderPassThru, this ),		"" );
	mParams.addButton( "Light shader",			bind( &VideoShadersApp::shaderLight, this ),		"" );
	mParams.addButton( "Edge Detection shader",	bind( &VideoShadersApp::shaderEdgeDetection, this ),"" );
	mParams.addButton( "Colors",				bind( &VideoShadersApp::shaderColors, this ),		"" );
	mParams.addButton( "Test shader",			bind( &VideoShadersApp::shaderTest, this ),			"" );

	// Video / Audio Infos
	mParams.addButton( "open", std::bind( &VideoShadersApp::open, this ) );
	mParams.addButton( "clear all", std::bind( &VideoShadersApp::clearAll, this ) );
	mParams.addParam("manual video buffer", &m_bUseVideoBuffer);
	mParams.addParam("manual audio buffer", &m_bUseAudioBuffer);
	mParams.addSeparator();
	strTmp << "label='file: '";
	mParams.addText("file", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='video codec: '";
	mParams.addText("video codec", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='audio codec: '";
	mParams.addText("audio codec", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='width: '";
	mParams.addText("width", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='height: '";
	mParams.addText("height", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='fps: '";
	mParams.addText("fps", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='bitrate: '";
	mParams.addText("bitrate", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='audio channels: '";
	mParams.addText("audio channels", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='audio sample rate: '";
	mParams.addText("audio sample rate", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='frame: '";
	mParams.addText("frame", strTmp.str());
	strTmp.clear();	strTmp.str("");
	strTmp << "label='time: '";
	mParams.addText("time", strTmp.str());
	mParams.addSeparator();
	mParams.addButton( "play/pause", std::bind( &VideoShadersApp::pause, this ) );
	mParams.addButton( "stop", std::bind( &VideoShadersApp::stop, this ) );
	mParams.addButton( "toggle direction", std::bind( &VideoShadersApp::toggleDirection, this ) );
	mParams.addSeparator();
	mParams.addParam("speed", &m_fSpeed, "min=0 max=8.0 step=0.05");
	mParams.addParam("0..none, 1..loop, 2..loopBidi", &m_iLoopMode, "min=0 max=2 step=1");
	mParams.addParam("seek frame", &m_fSeekPos, "min=0.0 max=1.0 step=0.01");
	mParams.addParam("volume", &m_fVolume, "min=0.0 max=1.0 step=0.01");
	mParams.addButton( "Create window",			bind( &VideoShadersApp::createNewWindow, this ),	"key=n" );
	mParams.addButton( "Delete windows",		bind( &VideoShadersApp::deleteWindows, this ),		"key=d" );
	mParams.addButton( "Quit",					bind( &VideoShadersApp::shutdown, this ),			"key=q" );


}

int	VideoShadersApp::calcTileDivisor(int size)
{
	float fTmp = sqrt(double(size));
	if((fTmp - int(fTmp))>0)
		fTmp++;
	return int(fTmp);
}

int VideoShadersApp::calcSelectedPlayer(int x, int y)
{
	int selected = int((float(x) / float(m_iTileWidth))) + m_iTilesDivisor * int( (float(y) / float(m_iTileHeight)));

	return selected;
}
void VideoShadersApp::open()
{
	fs::path moviePath = getOpenFilePath();
	if( ! moviePath.empty() )
	{
		std::shared_ptr<GStreamerWrapper> fileToLoad = std::shared_ptr<GStreamerWrapper>(new GStreamerWrapper());
		std::string uri = "file:/" + moviePath.string();
		if(fileToLoad->open(uri, m_bUseVideoBuffer, m_bUseAudioBuffer))
		{
			m_Players.push_back(fileToLoad);
			m_VideoTextures.push_back(gl::Texture());
			m_Players.back()->play();
		}
	}
}
void VideoShadersApp::mouseDown( MouseEvent event )
{
	iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
	if(event.isLeft())
	{
		int selected = calcSelectedPlayer(event.getX(), event.getY()); 
		if(selected < m_Players.size())
			m_iCurrentVideo = selected;
	}
	else if(event.isRight())
	{
		int selected = calcSelectedPlayer(event.getX(), event.getY()); 
		if(selected < m_Players.size())
		{
			m_iCurrentVideo = selected;
			m_Players.erase(m_Players.begin() + selected);		
			m_VideoTextures.erase(m_VideoTextures.begin() + selected);		
			if(m_iCurrentVideo>=m_Players.size())
				m_iCurrentVideo = m_Players.size() - 1;
			if(m_iCurrentVideo<=0)
				m_iCurrentVideo = 0;
		}
	}
}
void VideoShadersApp::update()
{
	// gStreamer : set playing properties of current video
	if(m_Players.size()<=0)
		return;

	m_Players[m_iCurrentVideo]->setSpeed(m_fSpeed);
	m_Players[m_iCurrentVideo]->setLoopMode((LoopMode)m_iLoopMode);
	m_Players[m_iCurrentVideo]->setVolume(m_fVolume);

	updateGui();

	// scrubbing through stream
	if(m_fSeekPos != m_fOldSeekPos)
	{
		m_fOldSeekPos = m_fSeekPos;
		m_Players[m_iCurrentVideo]->setPosition(m_fSeekPos);
	}

	m_iTilesDivisor = calcTileDivisor(m_Players.size());
	m_iTileWidth = getWindowWidth() / m_iTilesDivisor;
	m_iTileHeight = getWindowHeight() / m_iTilesDivisor;
	//mHotShader.update();
	iGlobalTime += 0.01;
	gui->update(); 
	mvg->addPoint(getAverageFps());
	fps->setLabel(ci::toString(getAverageFps()));
	
	while( receiver.hasWaitingMessages() ) {
		osc::Message m;
		receiver.getNextMessage( &m );

		console() << "New message received" << std::endl;
		console() << "Address: " << m.getAddress() << std::endl;
		console() << "Num Arg: " << m.getNumArgs() << std::endl;
		if(m.getAddress() == "/mouse/position")
		{
			iMouse = Vec3i( m.getArgAsInt32(0), m.getArgAsInt32(1), m.getArgAsInt32(2));
		}
		// check for mouse button message
		else if(m.getAddress() == "/mouse/button")
		{
			iMouse = Vec3i( m.getArgAsInt32(0), m.getArgAsInt32(1), m.getArgAsInt32(2));
		}
		else if(m.getAddress() == "/video/load"){
			fs::path imagePath = m.getArgAsString(0);
			addFullScreenMovie( imagePath );
		}	
		else if(m.getAddress() == "/shader/change"){
			switch (m.getArgAsInt32(0))
			{
			case 0:
				shaderPassThru();
				break;
			case 1:
				shaderLight();
				break;
			}
		}
		else if(m.getAddress() == "/quit"){
			quitProgram();
		}		
		else{
			// unrecognized message
			//cout << "not recognized:" << m.getAddress() << endl;
		}

	}
}

void VideoShadersApp::draw()
{
	// clear out the window with transparency
	gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );

	// Draw on render window only
	if (app::getWindow() == controlWindow)	
	{
		Rectf imgRect;
		int posX;
		int posY;

		//gl::clear();
		gl::draw( mTexture0, Rectf( getWindowWidth() - 320, 0, getWindowWidth() - 224, 96 ) );
		for(int i=0; i<m_Players.size(); i++)
		{
			m_Players[i]->update();
			if(m_Players[i]->hasVideo() && m_Players[i]->isNewVideoFrame())
			{	
				unsigned char* pImg = m_Players[i]->getVideo();
				if(pImg != nullptr)
				{		
					m_VideoTextures[i] = gl::Texture(ci::Surface(pImg, m_Players[i]->getWidth(), m_Players[i]->getHeight(), m_Players[i]->getWidth() * 3, ci::SurfaceChannelOrder::RGB) );
				}
			}

			posX = (i % m_iTilesDivisor) * m_iTileWidth;
			posY = ((int(float(i) / float(m_iTilesDivisor))) % m_iTilesDivisor) * m_iTileHeight;
			Rectf imgRect = Rectf(posX, posY, posX + m_iTileWidth, posY + m_iTileHeight);
			if(m_VideoTextures[i])
				ci::gl::draw( m_VideoTextures[i] , imgRect);
		}

		// draw green selection frame
		if(m_Players.size()>0)
		{
			posX = (m_iCurrentVideo % m_iTilesDivisor) * m_iTileWidth;
			posY = ((int(float(m_iCurrentVideo) / float(m_iTilesDivisor))) % m_iTilesDivisor) * m_iTileHeight;
			gl::color(0,1,0,1);
			glLineWidth(3);
			gl::drawStrokedRect(Rectf(posX, posY, posX + m_iTileWidth, posY + m_iTileHeight));
			gl::color(1,1,1,1);
		}

		// draw fps and gui
		std::stringstream str;
		str << getAverageFps();
		gl::drawString(str.str(), Vec2f( float(getWindowWidth() - 240), 10.0 ), Color(1,0,0), m_Font);

		// Draw the params on control window only
		mParams.draw();
		gui->draw();
		/*if( mInfoTexture ) 
		{
			glDisable( GL_TEXTURE_RECTANGLE_ARB );
			gl::draw( mInfoTexture, Vec2f( 20, getWindowHeight() - 20 - (float)mInfoTexture.getHeight() ) );
		}*/

	}
	else
	{
		gl::enableAlphaBlending();
		glEnable( GL_TEXTURE_2D );
		gl::color(Color::white());
		//gl::setMatricesWindow(mRenderWidth/2, mRenderHeight/2, false);
		gl::setMatricesWindow(mScreenWidth, mScreenHeight, mOriginUpperLeft);
		//gl::scale( Vec3f(1, -1, 1) );
		mShader.bind();
		mShader.uniform("iGlobalTime",iGlobalTime);
		mShader.uniform("iResolution",iResolution);
		mShader.uniform("iChannelResolution", iChannelResolution);
		mShader.uniform("iMouse", iMouse);
		mShader.uniform("iChannel0", 0);
		mShader.uniform("iChannel1", 1);
		mShader.uniform("iChannel2", 2);

		mShader.uniform("width",mTexture0.getWidth()); 
		mShader.uniform("height",mTexture0.getHeight()); 
		if(m_Players.size()>0)
		{
			if(m_VideoTextures[0])
			{
				m_VideoTextures[0].bind(0);
			}
			else
			{
				mTexture0.bind(0);
			}
		}
		else
		{
			mTexture0.bind(0);
		}
		mTexture0.bind(1);
		mTexture0.bind(2);

		gl::drawSolidRect(getWindowBounds());
		mTexture0.unbind();
		if(m_Players.size()>0)
		{
			if(m_VideoTextures[0])
			{
				m_VideoTextures[0].unbind(0);
			}
			Rectf imgRect = Rectf(40, 40, 200, 160);
			if(m_VideoTextures[0])
				ci::gl::draw( m_VideoTextures[0] , imgRect);
		}
		mShader.unbind();

		//glDisable( GL_TEXTURE_2D );
	}
}

CINDER_APP_NATIVE( VideoShadersApp, RendererGl )
