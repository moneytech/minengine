////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine/engine.hpp"

#include <GL/glew.h>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}; // extern "C"

#include <cstdio>
#include <cmath>

#include "GLCheck.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

////////////////////////////////////////////////////////////////////////////////

#include "Transformable3D.hpp"
#include "Camera.hpp"
#include "ClientModel.hpp"
#include "ClientObject.hpp"
#include "ResourceCache.hpp"
#include "ShaderCache.hpp"
#include "TextureCache.hpp"
#include "MusicCache.hpp"
#include "LightInfo.hpp"

////////////////////////////////////////////////////////////////////////////////

class Player {
    Camera mCamera;
    float mEyeHeight;
    glm::vec2 mLookDir;

    Physics::Body mBody;
    glm::vec3 mPosition;

    mutable glm::mat4 mTransform;
    mutable bool mNeedsUpdate;

public:
    Player();

    Camera &getCamera();
    const Camera &getCamera() const;

    Physics::Body &getBody();
    const Physics::Body &getBody() const;

    const glm::mat4 &getTransform() const;

    glm::vec3 getEyePosition() const;

    void setPosition(const glm::vec3 &position);
    glm::vec3 getPosition() const;

    void move(const glm::vec3 &offset);

    void setLook(const glm::vec2 &look);
    const glm::vec2 &getLook() const;

    void look(const glm::vec2 &look);

    void render() const;
};

////////////////////////////////////////////////////////////////////////////////

Player::Player(
): mCamera(
    90.0f, 16.0f/9.0f, 0.01f, 100.0f
), mEyeHeight(
    1.7f
), mLookDir(
), mNeedsUpdate(
    true
) {
    mBody.setBounds(BoundingVolume::capsule(0.4f, 1.77f));
}

Camera &Player::getCamera() {
    return mCamera;
}

const Camera &Player::getCamera() const {
    return mCamera;
}

Physics::Body &Player::getBody() {
    return mBody;
}

const Physics::Body &Player::getBody() const {
    return mBody;
}

const glm::mat4 &Player::getTransform() const {
    if (mNeedsUpdate) {
        mTransform = glm::translate(
            glm::rotate(
                glm::rotate(
                    glm::mat4(),
                    glm::radians(mLookDir.y),
                    glm::vec3(1.0f, 0.0f, 0.0f)
                ),
                glm::radians(mLookDir.x),
                glm::vec3(0.0f, 1.0f, 0.0f)
            ),
            -getEyePosition()
        );
        mNeedsUpdate = false;
    }
    return mTransform;
}

glm::vec3 Player::getPosition() const {
    return mPosition;
}

void Player::setPosition(const glm::vec3 &position) {
    mPosition = position;
    mNeedsUpdate = true;
}

glm::vec3 Player::getEyePosition() const {
    glm::vec3 position(getPosition());
    return glm::vec3(position.x, position.y + mEyeHeight, position.z);
}

void Player::setLook(const glm::vec2 &look) {
    mLookDir = look;
    mNeedsUpdate = true;
}

const glm::vec2 &Player::getLook() const {
    return mLookDir;
}

void Player::look(const glm::vec2 &look) {
    mLookDir += look;

    if (mLookDir.y > 90.0f) {
        mLookDir.y = 90.0f;
    } else if (mLookDir.y < -90.0f) {
        mLookDir.y = -90.0f;
    }
}

void Player::move(const glm::vec3 &move) {
    setPosition(getPosition() + glm::vec3(
        glm::rotate(glm::mat4(),
            -glm::radians(getLook().x),
            glm::vec3(0,1,0)
        ) * glm::vec4(move, 1)
    ));
}

void Player::render() const {
}

////////////////////////////////////////////////////////////////////////////////

class ChunkRenderer {
public:
    void render(sf::RenderTarget &target, const Chunk &chunk);
};

////////////////////////////////////////////////////////////////////////////////

void ChunkRenderer::render(sf::RenderTarget &target, const Chunk &chunk) {
}

////////////////////////////////////////////////////////////////////////////////

ShaderCache gShaderCache;
TextureCache gTextureCache;
std::vector<ClientObject*> gObjects;
std::vector<LightInfo*> gLights;

extern int luaopen_Light(lua_State *L);
extern int luaopen_Object(lua_State *L);

////////////////////////////////////////////////////////////////////////////////

class GameWindow : protected sf::RenderWindow {
    bool mFullscreen;
    bool mMouseLocked;
    bool mAllowQuit;
    bool mQuitting;
    bool mPaused;

    enum class ViewMode {
        Normal,
        InsideWireframe,
        Wireframe,
    };
    ViewMode mViewMode;

    sf::VideoMode mDesktopMode;
    sf::VideoMode mWindowMode;
    sf::String mWindowTitle;
    sf::Uint32 mWindowStyle;
    sf::Vector2i mWindowCenter;
    sf::ContextSettings mContextSettings;

    sf::Font mFont;
    sf::Text mDebugText;

    sf::Time mMinFrameLength;
    sf::Time mFrameDelay;
    sf::Time mTickLength;
    sf::Time mPlayTime;
    unsigned int mMaxTicksPerFrame;

    float mTicksPerSecond;
    float mFramesPerSecond;
    sf::Time mInputLength;
    sf::Time mUpdateLength;
    sf::Time mRenderLength;
    sf::Time mIdleLength;
    sf::Time mFrameLength;

    Player mPlayer;

    bool mZoom;

    ChunkData mTestChunkData;
    Chunk mTestChunk;

    ChunkRenderer mChunkRenderer;

public:
    GameWindow();

    void run();
    void quit();

protected:
    bool init();
    void quit(bool internal);

    bool initScene();

    void handleEvents();

    void handleInput(const sf::Time &delta);
    void handleEvent(const sf::Event &event);
    void update(const sf::Time &delta);
    void render();

    void start3D();
    void end3D();
    void start2D();
    void end2D();

    void setMousePosition(const sf::Vector2i &position);
    sf::Vector2i getMousePosition() const;
    void lockMouse();
    void unlockMouse();
};

////////////////////////////////////////////////////////////////////////////////

GameWindow::GameWindow(
): mFullscreen(
    false
), mMouseLocked(
    false
), mAllowQuit(
    true
), mQuitting(
    false
), mPaused(
    false
), mViewMode(
    ViewMode::Normal
), mWindowMode(
    1280, 720
), mWindowTitle(
    L"MinEngine Client"
), mWindowStyle(
    sf::Style::Default
), mMinFrameLength(
    sf::microseconds(0) // disabled
    //~ sf::microseconds(2083) // ~480fps
    //~ sf::microseconds(4167) // ~240fps
    //~ sf::microseconds(8333) // ~120fps
    //~ sf::microseconds(16667) // ~60fps
    //~ sf::microseconds(33333) // ~30fps
), mFrameDelay(
    //~ sf::microseconds(1000)
), mTickLength(
    sf::microseconds(20000) // 50000
), mMaxTicksPerFrame(
    5
), mZoom(
    false
), mTestChunk(
    Position(),
    &mTestChunkData
) {
}

void GameWindow::run() {
    sf::Clock clock;
    sf::Time tickAccum;

    size_t tickCount = 0;
    size_t frameCount = 0;
    sf::Time fpsAccum;
    sf::Time fpsInterval(sf::microseconds(1000000));

    mQuitting = false;

    if (!init()) {
        return;
    }

    mFrameLength = sf::Time::Zero;

    while (!mQuitting) {
        sf::Time delta = clock.restart();

        handleEvents();

        handleInput(delta);

        sf::Time inputTime = clock.getElapsedTime();

        tickAccum += delta;
        fpsAccum += delta;

        unsigned int frameTicks = mMaxTicksPerFrame;

        while (tickAccum >= mTickLength) {
            tickAccum -= mTickLength;

            if (frameTicks > 0) {
                update(mTickLength);
                tickCount += 1;
                frameTicks -= 1;
            }
        }

        sf::Time updateTime = clock.getElapsedTime();

        render();

        sf::Time renderTime = clock.getElapsedTime();

        display();

        sf::sleep(mFrameDelay);

        sf::Time endTime = clock.getElapsedTime();

        static const float lastFrameRatio = 0.4f;
        static const float nextFrameRatio = 0.6f;

        mInputLength = lastFrameRatio * mInputLength + nextFrameRatio * inputTime;
        mUpdateLength = lastFrameRatio * mUpdateLength + nextFrameRatio * (updateTime - inputTime);
        mRenderLength = lastFrameRatio * mRenderLength + nextFrameRatio * (renderTime - updateTime);
        mIdleLength = lastFrameRatio * mIdleLength + nextFrameRatio * (endTime - renderTime);
        mFrameLength = lastFrameRatio * mFrameLength + nextFrameRatio * endTime;

        frameCount += 1;

        if (fpsAccum >= fpsInterval) {
            mTicksPerSecond = float(tickCount) / fpsAccum.asSeconds();
            mFramesPerSecond = float(frameCount) / fpsAccum.asSeconds();
            tickCount = frameCount = 0;
            fpsAccum = sf::Time::Zero;

            mFrameDelay = mMinFrameLength - mInputLength - mUpdateLength - mRenderLength;
        }
    }

    close();
}

void GameWindow::quit() {
    quit(false);
}

bool GameWindow::init() {
    mContextSettings = sf::ContextSettings(
        24, 8,  // depth and stencil bits
        8,      // antialiasing level
        3, 3,   // OpenGL version (major, minor)
        sf::ContextSettings::Default
        //~ sf::ContextSettings::Core
    );

    mDesktopMode = sf::VideoMode::getDesktopMode();

    std::fprintf(stderr, "Desktop mode: %dx%d %dbpp\n",
                 mDesktopMode.width, mDesktopMode.height,
                 mDesktopMode.bitsPerPixel);

    create(mWindowMode, mWindowTitle, mWindowStyle, mContextSettings);

    const sf::ContextSettings &usedSettings = getSettings();
    sf::err() << "Using OpenGL " << usedSettings.majorVersion << '.' << usedSettings.minorVersion << ' ' <<
        ((usedSettings.attributeFlags & sf::ContextSettings::Core) ? "Core" : "Compat") <<
        ((usedSettings.attributeFlags & sf::ContextSettings::Debug) ? " (Debug)" : "") <<
        ".\n";

    mFont.loadFromFile("data/fonts/VeraMono.ttf");
    mDebugText.setFont(mFont);
    mDebugText.setCharacterSize(16);

    mPlayer.getCamera().setFOV(75.0f);
    mPlayer.setPosition(glm::vec3(0,0,0));

    glm::vec3 eye = mPlayer.getEyePosition();
    sf::err() << "mPlayer.getEyePosition() == " << eye.x << ',' << eye.y << ',' << eye.z << '\n';

    glewExperimental = true;
    GLChecked(glewInit());

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthFunc(GL_LESS));
    GLChecked(glEnable(GL_CULL_FACE));

    if (!initScene()) {
        return false;
    }

    if (hasFocus()) {
        lockMouse();
    }

    return true;
}

bool GameWindow::initScene() {
    for (LightInfo *light : gLights) {
        delete light;
    }
    gLights.clear();

    for (ClientObject *object : gObjects) {
        delete object->getModel()->getModel();
        delete object->getModel();
        delete object;
    }
    gObjects.clear();

    lua_State *L = luaL_newstate();

    if (!L) {
        return false;
    }

    luaL_openlibs(L);

    lua_pushcfunction(L, luaopen_Object);
    lua_pushliteral(L, "Object");

    if (lua_pcall(L, 1, 1, 0)) {
        const char *msg = lua_tostring(L, -1);
        if (msg) {
            sf::err() << "Lua error: " << msg << std::endl;
        }
        lua_close(L);
        return false;
    } else {
        if (lua_isnoneornil(L, -1)) {
            lua_pop(L, 1);
        } else {
            lua_setglobal(L, "Object");
        }
    }

    lua_pushcfunction(L, luaopen_Light);
    lua_pushliteral(L, "Light");

    if (lua_pcall(L, 1, 1, 0)) {
        const char *msg = lua_tostring(L, -1);
        if (msg) {
            sf::err() << "Lua error: " << msg << std::endl;
        }
        lua_close(L);
        return false;
    } else {
        if (lua_isnoneornil(L, -1)) {
            lua_pop(L, 1);
        } else {
            lua_setglobal(L, "Light");
        }
    }

    if (luaL_loadfile(L, "data/scripts/main.lua") || lua_pcall(L, 0, 0, 0)) {
        const char *msg = lua_tostring(L, -1);
        if (msg) {
            sf::err() << "Lua error: " << msg << std::endl;
        }
        lua_close(L);
        return false;
    }

    lua_close(L);

    return true;
}

void GameWindow::quit(bool internal) {
    if (internal || mAllowQuit) {
        mQuitting = true;
    }
}

void GameWindow::handleEvents() {
    sf::Event event;

    while (pollEvent(event)) {
        handleEvent(event);
    }
}

void GameWindow::handleEvent(const sf::Event &event) {
    switch (event.type) {
        case sf::Event::Closed: {
            quit();
            break;
        }

        case sf::Event::Resized: {
            sf::Vector2f size(getSize());
            sf::err() << "Window resized to " << size.x << 'x' << size.y << '\n';
            GLChecked(glViewport(0, 0, size.x, size.y));
            if (size.y > 0) {
                mPlayer.getCamera().setAspect(size.x / size.y);
            }
            mWindowCenter = sf::Vector2i(size) / 2;
            break;
        }

        case sf::Event::LostFocus: {
            break;
        }

        case sf::Event::GainedFocus: {
            break;
        }

        case sf::Event::TextEntered: {
            break;
        }

        case sf::Event::KeyPressed: {
            /// @note for debugging
            switch (event.key.code) {
                case sf::Keyboard::Escape: {
                    quit();
                    break;
                }

                case sf::Keyboard::Tab: {
                    if (mMouseLocked) {
                        unlockMouse();
                    } else {
                        lockMouse();
                    }
                    break;
                }

                case sf::Keyboard::Space: {
                    mPaused = !mPaused;
                    break;
                }

                case sf::Keyboard::Num1: {
                    mViewMode = ViewMode::Normal;
                    break;
                }

                case sf::Keyboard::Num2: {
                    mViewMode = ViewMode::InsideWireframe;
                    break;
                }

                case sf::Keyboard::Num3: {
                    mViewMode = ViewMode::Wireframe;
                    break;
                }

                case sf::Keyboard::F11: {
                    bool wasLocked = mMouseLocked;

                    if (wasLocked) {
                        unlockMouse();
                    }

                    if (mFullscreen) {
                        create(mWindowMode, mWindowTitle, mWindowStyle, mContextSettings);
                    } else {
                        create(mDesktopMode, mWindowTitle, mWindowStyle | sf::Style::Fullscreen, mContextSettings);
                    }

                    mFullscreen = !mFullscreen;

                    if (wasLocked) {
                        lockMouse();
                    }

                    break;
                }

                case sf::Keyboard::R: {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        gTextureCache.reloadAll();
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                        gShaderCache.reloadAll();
                    }
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
                        initScene();
                    }
                    break;
                }

                default: {
                    break;
                }
            }
            break;
        }

        case sf::Event::KeyReleased: {
            break;
        }

        case sf::Event::MouseWheelMoved: {
            break;
        }

        case sf::Event::MouseWheelScrolled: {
            if (event.mouseWheelScroll.delta > 0) {
                mZoom = true;
            } else if (event.mouseWheelScroll.delta < 0) {
                mZoom = false;
            }
            break;
        }

        case sf::Event::MouseButtonPressed: {
            break;
        }

        case sf::Event::MouseButtonReleased: {
            break;
        }

        case sf::Event::MouseMoved: {
            break;
        }

        case sf::Event::MouseEntered: {
            break;
        }

        case sf::Event::MouseLeft: {
            break;
        }

        case sf::Event::JoystickButtonPressed: {
            break;
        }

        case sf::Event::JoystickButtonReleased: {
            break;
        }

        case sf::Event::JoystickMoved: {
            if (std::fabs(event.joystickMove.position) >= 10.0f) {
                fprintf(stderr, "%d:%d: %.2f\n",
                        event.joystickMove.joystickId,
                        event.joystickMove.axis,
                        event.joystickMove.position);

                switch (event.joystickMove.axis) {
                    case sf::Joystick::X: {
                        //~ mPlayer.move(event.joystickMove.position * 0.01f, 0, 0);
                        break;
                    }

                    case sf::Joystick::Y: {
                        //~ mPlayer.move(0, 0, event.joystickMove.position * 0.01f);
                        break;
                    }

                    case sf::Joystick::Z: {
                        break;
                    }

                    case sf::Joystick::R: {
                        break;
                    }

                    case sf::Joystick::U: {
                        break;
                    }

                    case sf::Joystick::V: {
                        break;
                    }

                    case sf::Joystick::PovX: {
                        break;
                    }

                    case sf::Joystick::PovY: {
                        break;
                    }
                }
            }
            break;
        }

        case sf::Event::JoystickConnected: {
            break;
        }

        case sf::Event::JoystickDisconnected: {
            break;
        }

        case sf::Event::TouchBegan: {
            sf::err() << "T-down " << event.touch.finger << ':' <<
                event.touch.x << ',' << event.touch.y << std::endl;
            break;
        }

        case sf::Event::TouchMoved: {
            sf::err() << "T-move " << event.touch.finger << ':' <<
                event.touch.x << ',' << event.touch.y << std::endl;
            break;
        }

        case sf::Event::TouchEnded: {
            sf::err() << "T-up   " << event.touch.finger << ':' <<
                event.touch.x << ',' << event.touch.y << std::endl;
            break;
        }

        case sf::Event::SensorChanged: {
            break;
        }

        // silence "enumeration value not handled" warning
        default: {
            break;
        }
    }
}

void GameWindow::handleInput(const sf::Time &delta) {
    if (hasFocus()) {
        glm::vec2 look;

        if (mMouseLocked) {
            sf::Vector2i mousePos = getMousePosition();
            sf::Vector2f mouseDelta(mousePos - mWindowCenter);
            look = glm::vec2(mouseDelta.x, mouseDelta.y);
            setMousePosition(mWindowCenter);
        }

        float deltaSeconds = delta.asSeconds();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            look.x -= 180*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            look.x += 180*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            look.y -= 180*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            look.y += 180*deltaSeconds;
        }

        glm::vec3 move;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            move.z -= 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            move.z += 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            move.x -= 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            move.x += 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            move.y -= 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            move.y += 2*deltaSeconds;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            look *= 0.25f;
            move *= 0.25f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            look *= 4.0f;
            move *= 4.0f;
        }

        float fov = mPlayer.getCamera().getFOV();

        if (mZoom) {
            look *= 0.5f;
            if (fov > 30.0f) {
                fov -= 180.0f * delta.asSeconds();
            }
        } else {
            if (fov < 75.0f) {
                fov += 180.0f * delta.asSeconds();
            }
        }

        mPlayer.getCamera().setFOV(fov);

        mPlayer.look(look);
        mPlayer.move(move);
    }
}

void GameWindow::update(const sf::Time &delta) {
    if (!mPaused) {
        mPlayTime += delta;
    }
}

void GameWindow::start3D() {
    GLChecked(glEnable(GL_DEPTH_TEST));

    switch (mViewMode) {
        default:
        case ViewMode::Normal: {
            GLChecked(glEnable(GL_CULL_FACE));
            break;
        }

        case ViewMode::InsideWireframe: {
            GLChecked(glPolygonMode(GL_BACK, GL_LINE));
            break;
        }

        case ViewMode::Wireframe: {
            GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            break;
        }
    }
}

void GameWindow::end3D() {
    // nothing to do here
}

void GameWindow::start2D() {
    GLChecked(glDisable(GL_DEPTH_TEST));
    GLChecked(glDisable(GL_CULL_FACE));
    GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GLChecked(pushGLStates());
}

void GameWindow::end2D() {
    GLChecked(popGLStates());
}

void GameWindow::render() {
    char temp[256];
    glm::vec3 p = mPlayer.getPosition();
    glm::vec3 e = mPlayer.getEyePosition();
    glm::vec2 o = mPlayer.getLook();
    snprintf(temp, sizeof(temp),
             "%.2ffps (%lldus/f, %lldus delay) / %.2ftps\n"
             "%8.4f,%8.4f,%8.4f (%8.4f,%8.4f,%8.4f)\n"
             "%8.4f,%8.4f",
             mFramesPerSecond, mFrameLength.asMicroseconds(), mFrameDelay.asMicroseconds(), mTicksPerSecond,
             p.x, p.y, p.z, e.x, e.y, e.z, o.x, o.y);
    mDebugText.setString(temp);

    GLChecked(glClearColor(1,0,1,0));
    GLChecked(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////
    //  3D setup
    ////////////////////////////////////////////////////////////

    start3D();

    // draw 3D scene

    /**
     * @todo
     * for each visible chunk:
     *      create vbo
     *      convert blocks to polys
     *      render vbo
     */

    //! camera.render();

    mPlayer.render();

    glm::mat4 projectionTransform(mPlayer.getCamera().getTransform());
    glm::mat4 modelViewTransform(mPlayer.getTransform());
    glm::mat4 normalTransform(glm::transpose(glm::inverse(modelViewTransform)));

    static LightInfo defaultLight;

    for (LightInfo *light : gLights) {
        light->spotConeInnerCos = glm::cos(glm::radians(light->spotConeInner));
        light->spotConeOuterCos = glm::cos(glm::radians(light->spotConeOuter));
    }

    char paramName[32];

    for (ClientObject *object : gObjects) {
        sf::Shader *shader = object->getShader();
        if (shader) {
            sf::Shader::bind(shader);
            unsigned int shaderProgram = shader->getNativeHandle();
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjMatrix"), 1, 0, &projectionTransform[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uViewMatrix"), 1, 0, &modelViewTransform[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uNormMatrix"), 1, 0, &normalTransform[0][0]);

            for (size_t i = 0; i < 4; i++) {
                LightInfo *light = (i < gLights.size() && gLights[i]) ? gLights[i] : &defaultLight;
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "ambtColor");
                glUniform4fv(glGetUniformLocation(shaderProgram, paramName), 1, &light->ambtColor[0]);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "diffColor");
                glUniform4fv(glGetUniformLocation(shaderProgram, paramName), 1, &light->diffColor[0]);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "specColor");
                glUniform4fv(glGetUniformLocation(shaderProgram, paramName), 1, &light->specColor[0]);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "position");
                glm::vec4 position = modelViewTransform * light->position;
                glUniform4fv(glGetUniformLocation(shaderProgram, paramName), 1, &position[0]);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "spotDirection");
                glUniform3fv(glGetUniformLocation(shaderProgram, paramName), 1, &light->spotDirection[0]);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "spotExponent");
                shader->setParameter(paramName, light->spotExponent);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "spotConeInner");
                shader->setParameter(paramName, light->spotConeInner);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "spotConeInnerCos");
                shader->setParameter(paramName, light->spotConeInnerCos);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "spotConeOuter");
                shader->setParameter(paramName, light->spotConeOuter);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "spotConeOuterCos");
                shader->setParameter(paramName, light->spotConeOuterCos);
                snprintf(paramName, sizeof(paramName), "uLights[%d].%s", i, "attenuation");
                shader->setParameter(paramName, light->attenuation[0], light->attenuation[1], light->attenuation[2]);
            }
        }
        object->render();
    }

    end3D();

    ////////////////////////////////////////////////////////////
    //  end 3D
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    //  2D setup
    ////////////////////////////////////////////////////////////

    start2D();

    // draw 2D overlay

    float frameLength = mFrameLength.asSeconds();
    float inputFrac = mInputLength.asSeconds() / frameLength;
    float updateFrac = mUpdateLength.asSeconds() / frameLength;
    float renderFrac = mRenderLength.asSeconds() / frameLength;
    float idleFrac = 1.0f - inputFrac - updateFrac - renderFrac;

    sf::RectangleShape rect(sf::Vector2f(32, 32));
    rect.setFillColor(sf::Color::White);
    GLChecked(draw(rect));

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac + updateFrac + renderFrac)));
    rect.setFillColor(sf::Color::Red);
    GLChecked(draw(rect));

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac + renderFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac + updateFrac)));
    rect.setFillColor(sf::Color::Green);
    GLChecked(draw(rect));

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac + renderFrac + updateFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac)));
    rect.setFillColor(sf::Color::Blue);
    GLChecked(draw(rect));

    mDebugText.setColor(sf::Color::Black);
    mDebugText.setPosition(sf::Vector2f(33, 1));
    GLChecked(draw(mDebugText));
    mDebugText.setColor(sf::Color::White);
    mDebugText.setPosition(sf::Vector2f(32, 0));
    GLChecked(draw(mDebugText));

    end2D();

    ////////////////////////////////////////////////////////////
    //  end 2D
    ////////////////////////////////////////////////////////////
}

void GameWindow::setMousePosition(const sf::Vector2i &position) {
    sf::Mouse::setPosition(position, *this);
}

sf::Vector2i GameWindow::getMousePosition() const {
    return sf::Mouse::getPosition(*this);
}

void GameWindow::lockMouse() {
    mMouseLocked = true;
    mWindowCenter = sf::Vector2i(getSize()) / 2;
    setMousePosition(mWindowCenter);
}

void GameWindow::unlockMouse() {
    mMouseLocked = false;
}

////////////////////////////////////////////////////////////////////////////////

extern "C"
int main(int argc, char **argv) {

    // mine::Engine engine;

    //~ ChunkGenerator chunkGen;
    //~ ChunkCache chunkCache(chunkGen);

    //~ for (size_t i = 0; i < 4098; i++) {
        //~ chunkCache.getChunk(Position(0,0,0));
    //~ }

    //~ Chunk *chunk = chunkCache.getChunk(Position(0,0,0));

    //~ printf("chunk == %p, chunk->getData() == %p\n", chunk, chunk->getData());

    GameWindow gameWindow;

    gameWindow.run();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

