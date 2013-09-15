#include "VideoShadersApp.h"

//full screen movie
void VideoShadersApp::addFullScreenMovie( const fs::path &path )
{
	console() << "Add FullScreen Movie" << std::endl;
	try 
	{
		mMovie = qtime::MovieGl(path );
		mMovie.setLoop();
		mMovie.play();
		mMovie.setVolume( 0.0f );
		movieLoaded = true;
		// create a texture for showing some info about the movie
		TextLayout infoText;
		infoText.clear( ColorA( 0.2f, 0.2f, 0.2f, 0.5f ) );
		infoText.setColor( Color::white() );
		infoText.addCenteredLine( path.filename().string() );
		infoText.addLine( toString( mMovie.getWidth() ) + " x " + toString( mMovie.getHeight() ) + " pixels" );
		infoText.addLine( toString( mMovie.getDuration() ) + " seconds" );
		infoText.addLine( toString( mMovie.getNumFrames() ) + " frames" );
		infoText.addLine( toString( mMovie.getFramerate() ) + " fps" );
		infoText.setBorder( 4, 2 );
		mInfoTexture = gl::Texture( infoText.render( true ) );

	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie.reset();
		mInfoTexture.reset();
	}

	mFrameTexture.reset();
}
void VideoShadersApp::fileDrop( FileDropEvent event )
{
	addFullScreenMovie( event.getFile( 0 ) );
}
void VideoShadersApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getChar() == '1' )
		mMovie.setRate( 0.5f );
	else if( event.getChar() == '2' )
		mMovie.setRate( 12 );
}
void VideoShadersApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 450, 400 );
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

	movieLoaded = false;
	try 
	{
		iResolution = Vec3i( mRenderWidth, mRenderHeight, 1 );
		iGlobalTime = 1;
		iMouse = Vec3i( mRenderWidth/2, mRenderHeight/2, 1 );

		gl::Texture::Format format;
		format.setTargetRect();
		mTexture0 = gl::Texture(loadImage( loadAsset("background.jpg") ), format);
		iResolution = Vec3i( mTexture0.getWidth(), mTexture0.getHeight(), 1 );
		iChannelResolution = Vec3i( mTexture0.getWidth(),  mTexture0.getHeight(), 1);
		// load and compile the shader
		mShader = gl::GlslProg( loadAsset("deflt.vert"), loadAsset("passthru.frag") );
	}
	catch( const std::exception &e ) 
	{
		// if anything went wrong, show it in the output window
		console() << e.what() << std::endl;
	}
	mParams = params::InterfaceGl( "Params", Vec2i( 400, 300 ) );
	mParams.addParam( "Render Window X",		&mRenderX,											"" );
	mParams.addParam( "Render Window Y",		&mRenderY,											"" );
	mParams.addParam( "Render Window Width",	&mRenderWidth,										"" );
	mParams.addParam( "Render Window Height",	&mRenderHeight,										"" );
	mParams.addButton( "Pass Through",			bind( &VideoShadersApp::shaderPassThru, this ),		"" );
	mParams.addButton( "Light shader",			bind( &VideoShadersApp::shaderLight, this ),		"" );
	mParams.addButton( "Edge Detection shader",	bind( &VideoShadersApp::shaderEdgeDetection, this ),"" );
	mParams.addButton( "Create window",			bind( &VideoShadersApp::createNewWindow, this ),	"key=n" );
	mParams.addButton( "Delete windows",		bind( &VideoShadersApp::deleteWindows, this ),		"key=d" );
	mParams.addButton( "Quit",					bind( &VideoShadersApp::shutdown, this ),			"key=q" );

	//store window
	controlWindow = this->getWindow();
	int uniqueId = getNumWindows();
	controlWindow->getSignalClose().connect(
		[uniqueId,this] { shutdown(); this->console() << "You quit console window #" << uniqueId << std::endl; }
	);
	receiver.setup( 10009 );
	createNewWindow();
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
	VideoShadersApp::quit();
}
void VideoShadersApp::quitProgram()
{
	shutdown();
}
void VideoShadersApp::mouseDown( MouseEvent event )
{
	iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
}
void VideoShadersApp::mouseDrag( MouseEvent event )
{
	iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
}
void VideoShadersApp::mouseMove( MouseEvent event )
{
	iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
}
void VideoShadersApp::update()
{
	iGlobalTime += 0.01;
	if ( mMovie ) mFrameTexture = mMovie.getTexture();
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
		// Draw the params on control window only
		mParams.draw();
	}
	else
	{
		gl::enableAlphaBlending();
		glEnable( GL_TEXTURE_RECTANGLE_ARB );

		mShader.bind();
		mShader.uniform("iGlobalTime",iGlobalTime);
		mShader.uniform("iResolution",iResolution);
		mShader.uniform("iChannelResolution", iChannelResolution);
		mShader.uniform("iMouse", iMouse);
		mShader.uniform("iChannel0", 0);

		if ( mMovie && mFrameTexture )
		{
			mFrameTexture.bind(0);
			mShader.uniform("width",mFrameTexture.getWidth()); 
			mShader.uniform("height",mFrameTexture.getHeight()); 
		}
		else
		{
			mTexture0.bind(0);
			mShader.uniform("width",mTexture0.getWidth()); 
			mShader.uniform("height",mTexture0.getHeight()); 
		}

		gl::drawSolidRect(getWindowBounds());

		// unbind textures and shader
		if ( mFrameTexture ) mFrameTexture.unbind();
		else mTexture0.unbind();
		mShader.unbind();
	}
}

CINDER_APP_NATIVE( VideoShadersApp, RendererGl )
