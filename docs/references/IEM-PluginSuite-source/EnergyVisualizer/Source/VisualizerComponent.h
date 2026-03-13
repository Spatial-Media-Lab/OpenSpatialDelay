/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "../../resources/heatmap.h"
#include "../../resources/viridis_cropped.h"
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class VisualizerComponent : public juce::Component, public juce::OpenGLRenderer, private juce::Timer
{
public:
    VisualizerComponent()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

        openGLContext.setComponentPaintingEnabled (true);
        openGLContext.setContinuousRepainting (false);
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);

        addAndMakeVisible (&hammerAitovGrid);

        visualizedRMS.resize (nSamplePoints);

        startTimer (20);
    }

    ~VisualizerComponent()
    {
        openGLContext.detach();
        openGLContext.setRenderer (nullptr);
    }

    void timerCallback() override { openGLContext.triggerRepaint(); }

    void setRmsDataPtr (float* rmsPtr) { pRMS = rmsPtr; }

    void newOpenGLContextCreated() override { createShaders(); }

    void setPeakLevel (const float newPeakLevel) { peakLevel = newPeakLevel; }

    void setDynamicRange (const float newDynamicRange) { dynamicRange = newDynamicRange; }

    void setHoldMax (const bool newHoldMax) { holdMax = newHoldMax; }

    void renderOpenGL() override
    {
        using namespace juce;
        using namespace juce::gl;

        jassert (juce::OpenGLHelpers::isContextActive());

        juce::OpenGLHelpers::clear (juce::Colour (0xFF2D2D2D));

        const float desktopScale = (float) openGLContext.getRenderingScale();
        glViewport (-5,
                    -5,
                    juce::roundToInt (desktopScale * getWidth() + 10),
                    juce::roundToInt (desktopScale * getHeight() + 10));

        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);

        texture.bind();
        glTexParameteri (GL_TEXTURE_2D,
                         GL_TEXTURE_MIN_FILTER,
                         GL_LINEAR); // linear interpolation when too small
        glTexParameteri (GL_TEXTURE_2D,
                         GL_TEXTURE_MAG_FILTER,
                         GL_LINEAR); // linear interpolation when too bi

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear (GL_COLOR_BUFFER_BIT);

        shader->use();

        if (firstRun)
        {
            juce::PixelARGB colormapData[512];
            for (int i = 0; i < 256; ++i)
            {
                const float alpha = juce::jlimit (0.0f, 1.0f, (float) i / 50.0f);
                colormapData[i] = juce::Colour::fromFloatRGBA (viridis_cropped[i][0],
                                                               viridis_cropped[i][1],
                                                               viridis_cropped[i][2],
                                                               alpha)
                                      .getPixelARGB();
                colormapData[256 + i] = juce::Colour::fromFloatRGBA (heatmap[i][0],
                                                                     heatmap[i][1],
                                                                     heatmap[i][2],
                                                                     heatmap[i][3])
                                            .getPixelARGB();
            }
            texture.loadARGB (colormapData, 256, 2);

            firstRun = false;
            openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
            openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
            openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                                   sizeof (hammerAitovSampleVertices),
                                                   hammerAitovSampleVertices,
                                                   GL_STATIC_DRAW);

            openGLContext.extensions.glGenBuffers (1, &indexBuffer);
            openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                                   sizeof (hammerAitovSampleIndices),
                                                   hammerAitovSampleIndices,
                                                   GL_STATIC_DRAW);
        }

        static GLfloat g_colorMap_data[nSamplePoints];
        for (int i = 0; i < nSamplePoints; i++)
        {
            if (holdMax)
                visualizedRMS[i] = std::max (pRMS[i], visualizedRMS[i]);
            else
                visualizedRMS[i] = pRMS[i];

            const float val =
                (juce::Decibels::gainToDecibels (visualizedRMS[i]) - peakLevel) / dynamicRange
                + 1.0f;
            g_colorMap_data[i] = juce::jlimit (0.0f, 1.0f, val);
        }

        GLuint colorBuffer;
        openGLContext.extensions.glGenBuffers (1, &colorBuffer);
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, colorBuffer);
        openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                               sizeof (g_colorMap_data),
                                               g_colorMap_data,
                                               GL_STATIC_DRAW);

        if (colormapChooser != nullptr)
            colormapChooser->set (usePerceptualColormap ? 0.0f : 1.0f);

        GLint attributeID;
        GLint programID = shader->getProgramID();

        // 1st attribute buffer : vertices
        attributeID = openGLContext.extensions.glGetAttribLocation (programID, "position");
        openGLContext.extensions.glEnableVertexAttribArray (attributeID);
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        openGLContext.extensions.glVertexAttribPointer (
            attributeID, // attribute 0. No particular reason for 0, but must match the layout in the shader.
            2, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*) 0 // array buffer offset
        );

        // 2nd attribute buffer : colors
        attributeID = openGLContext.extensions.glGetAttribLocation (programID, "colormapDepthIn");

        openGLContext.extensions.glEnableVertexAttribArray (attributeID);
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, colorBuffer);

        openGLContext.extensions.glVertexAttribPointer (
            attributeID, // attribute. No particular reason for 1, but must match the layout in the shader.
            1, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*) 0 // array buffer offset
        );

        //glDrawElements (GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);  // Draw triangles!
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        //        glPointSize(20.0);
        //        glDrawElements (GL_POINTS, 100, GL_UNSIGNED_INT, 0);  // Draw triangles!
        glDrawElements (GL_TRIANGLES, // mode
                        sizeof (hammerAitovSampleIndices), // count
                        GL_UNSIGNED_INT, // type
                        (void*) 0 // element array buffer offset
        );

        openGLContext.extensions.glDisableVertexAttribArray (0);
        openGLContext.extensions.glDisableVertexAttribArray (1);

        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

        openGLContext.extensions.glDeleteBuffers (1, &colorBuffer);
    }
    void openGLContextClosing() override
    {
        openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
        openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
        texture.release();
    }

    void paint (juce::Graphics& g) override {}

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        hammerAitovGrid.setBounds (getLocalBounds());
    }

    void createShaders()
    {
        //        vertexShader =
        //        "attribute vec3 position;\n"
        //        "void main()\n"
        //        "{\n"
        //        "   gl_Position.xyz = position;\n"
        //        "   gl_Position.w = 1.0;\n"
        //        "}";
        //

        //        fragmentShader =
        //        "void main()\n"
        //        "{\n"
        //        "      gl_FragColor = vec4(1,0,0,0.5);\n"
        //        "}";

        vertexShader = "attribute vec2 position;\n"
                       "attribute float colormapDepthIn;\n"
                       "uniform float colormapChooser;\n"
                       "varying float colormapChooserOut;\n"
                       "varying float colormapDepthOut;\n"
                       "void main()\n"
                       "{\n"
                       "   gl_Position.xy = position;\n"
                       "   gl_Position.z = 0.0;\n"
                       "   gl_Position.w = 1.0;\n"
                       "   colormapDepthOut = colormapDepthIn;\n"
                       "   colormapChooserOut = colormapChooser;\n"
                       "}";

        fragmentShader =
            "varying float colormapDepthOut;\n"
            "varying float colormapChooserOut;\n"
            "uniform sampler2D tex0;\n"
            "void main()\n"
            "{\n"
            //"      gl_FragColor = fragmentColor;\n"
            "      gl_FragColor = texture2D(tex0, vec2(colormapDepthOut, colormapChooserOut));\n"
            "}";

        std::unique_ptr<juce::OpenGLShaderProgram> newShader (
            new juce::OpenGLShaderProgram (openGLContext));
        juce::String statusText;

        if (newShader->addVertexShader (
                juce::OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
            && newShader->addFragmentShader (
                juce::OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
            && newShader->link())
        {
            shader = std::move (newShader);
            shader->use();

            colormapChooser.reset (createUniform (openGLContext, *shader, "colormapChooser"));
            statusText =
                "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
        }
        else
        {
            statusText = newShader->getLastError();
        }
    }

    void setColormap (bool shouldUsePerceptualColormap)
    {
        usePerceptualColormap = shouldUsePerceptualColormap;
    }

private:
    HammerAitovGrid hammerAitovGrid;
    GLuint vertexBuffer, indexBuffer;
    const char* vertexShader;
    const char* fragmentShader;
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> colormapChooser;
    bool usePerceptualColormap = true;

    std::vector<float> visualizedRMS;

    static juce::OpenGLShaderProgram::Uniform*
        createUniform (juce::OpenGLContext& openGLContext,
                       juce::OpenGLShaderProgram& shaderProgram,
                       const char* uniformName)
    {
        if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(),
                                                           uniformName)
            < 0)
            return nullptr; // Return if error
        return new juce::OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
    }

    float peakLevel = 0.0f; // dB
    float dynamicRange = 35.0f; // dB

    juce::OpenGLTexture texture;

    bool firstRun = true;
    bool holdMax = false;

    float* pRMS;

    juce::OpenGLContext openGLContext;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizerComponent)
};
