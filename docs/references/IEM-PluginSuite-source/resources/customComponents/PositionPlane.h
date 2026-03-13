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

class PositionPlane : public juce::Component
{
public:
    PositionPlane() : drawPlane (xy), autoScale (true), dimensions (1.0f, 1.0f, 1.0f) {};

    ~PositionPlane() { deleteAllChildren(); };

    enum Planes
    {
        xy,
        zy,
        zx
    };

    class Element
    {
    public:
        Element() {}
        Element (juce::String newID) { ID = newID; }
        virtual ~Element() {}

        virtual void startMovement() {};
        virtual void moveElement (const juce::MouseEvent& event,
                                  const juce::Point<float> centre,
                                  const float scale,
                                  Planes plane,
                                  PositionPlane* positionPlane,
                                  int xFactor = 1,
                                  int yFactor = 1,
                                  int zFactor = 1) = 0;
        virtual void stopMovement() {};

        void setActive (bool shouldBeActive) { active = shouldBeActive; }
        bool isActive() { return active; }

        void setColour (juce::Colour newColour) { faceColour = newColour; }
        juce::Colour getColour() { return faceColour; }

        void setLabel (juce::String newLabel) { label = newLabel; }
        void setID (juce::String newID) { ID = newID; }

        virtual juce::Vector3D<float> getPosition() = 0;

        juce::String getLabel() { return label; };
        juce::String getID() { return ID; };

        void addPlane (PositionPlane* positionPlane)
        {
            jassert (positionPlane != nullptr);
            if (positionPlane != nullptr)
                planesImIn.add (positionPlane);
        };
        void removePlane (PositionPlane* positionPlane)
        {
            planesImIn.removeFirstMatchingValue (positionPlane);
        };

        void repaintAllPlanesImIn()
        {
            for (int i = planesImIn.size(); --i >= 0;)
            {
                PositionPlane* handle = planesImIn.getUnchecked (i);
                handle->repaint();
            }
        }

    private:
        bool active = true;

        juce::Colour faceColour = juce::Colours::white;
        juce::String ID = "";
        juce::String label = "";

        juce::Array<PositionPlane*> planesImIn;
    };

    class ParameterElement : public Element
    {
    public:
        ParameterElement (juce::AudioProcessorParameter& xParameter,
                          juce::NormalisableRange<float> xParameterRange,
                          juce::AudioProcessorParameter& yParameter,
                          juce::NormalisableRange<float> yParameterRange,
                          juce::AudioProcessorParameter& zParameter,
                          juce::NormalisableRange<float> zParameterRange) :
            x (xParameter),
            xRange (xParameterRange),
            y (yParameter),
            yRange (yParameterRange),
            z (zParameter),
            zRange (zParameterRange)
        {
        }

        void startMovement() override
        {
            x.beginChangeGesture();
            y.beginChangeGesture();
            z.beginChangeGesture();
        };

        void moveElement (const juce::MouseEvent& event,
                          const juce::Point<float> centre,
                          const float scale,
                          Planes plane,
                          PositionPlane* positionPlane,
                          int xFactor = 1,
                          int yFactor = 1,
                          int zFactor = 1) override
        {
            auto mousePos = event.getPosition().toFloat();
            mousePos.x -= centre.x;
            mousePos.y -= centre.y;
            mousePos /= scale;

            juce::Vector3D<float> roomDims = positionPlane->getDimensions();
            juce::Vector3D<float> pos;

            switch (plane)
            {
                case xy:
                    pos.x = -mousePos.y * xFactor;
                    pos.y = -mousePos.x * yFactor;
                    pos.x =
                        juce::Range<float> (-0.5 * roomDims.x, 0.5 * roomDims.x).clipValue (pos.x);
                    pos.y =
                        juce::Range<float> (-0.5 * roomDims.y, 0.5 * roomDims.y).clipValue (pos.y);
                    x.setValueNotifyingHost (xRange.convertTo0to1 (pos.x));
                    y.setValueNotifyingHost (yRange.convertTo0to1 (pos.y));

                    break;
                case zy:
                    pos.z = -mousePos.y * zFactor;
                    pos.y = -mousePos.x * yFactor;
                    pos.z =
                        juce::Range<float> (-0.5 * roomDims.z, 0.5 * roomDims.z).clipValue (pos.z);
                    pos.y =
                        juce::Range<float> (-0.5 * roomDims.y, 0.5 * roomDims.y).clipValue (pos.y);
                    z.setValueNotifyingHost (zRange.convertTo0to1 (pos.z));
                    y.setValueNotifyingHost (yRange.convertTo0to1 (pos.y));
                    break;
                case zx:
                    pos.z = -mousePos.y * zFactor;
                    pos.x = mousePos.x * xFactor;
                    pos.z =
                        juce::Range<float> (-0.5 * roomDims.z, 0.5 * roomDims.z).clipValue (pos.z);
                    pos.x =
                        juce::Range<float> (-0.5 * roomDims.x, 0.5 * roomDims.x).clipValue (pos.x);
                    z.setValueNotifyingHost (zRange.convertTo0to1 (pos.z));
                    x.setValueNotifyingHost (xRange.convertTo0to1 (pos.x));
                    break;
            }
        }

        void stopMovement() override
        {
            x.endChangeGesture();
            y.endChangeGesture();
            z.endChangeGesture();
        };

        /**
         Get cartesian coordinates
         */
        juce::Vector3D<float> getPosition() override
        {
            return juce::Vector3D<float> (xRange.convertFrom0to1 (x.getValue()),
                                          yRange.convertFrom0to1 (y.getValue()),
                                          zRange.convertFrom0to1 (z.getValue()));
        };

    private:
        juce::AudioProcessorParameter& x;
        juce::NormalisableRange<float> xRange;
        juce::AudioProcessorParameter& y;
        juce::NormalisableRange<float> yRange;
        juce::AudioProcessorParameter& z;
        juce::NormalisableRange<float> zRange;
    };

    class PositionPlaneListener
    {
    public:
        virtual ~PositionPlaneListener() {}

        virtual void PositionPlaneElementChanged (PositionPlane* plane, Element* element) = 0;
    };

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<float> bounds (0, 0, getBounds().getWidth(), getBounds().getHeight());
        float innerSpacing = 3.0f;
        bounds.reduce (innerSpacing, innerSpacing);

        const float width = bounds.getWidth();
        const float height = bounds.getHeight();

        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();

        float drawH;
        float drawW;

        const int xFactor = xFlip ? -1 : 1;
        const int yFactor = yFlip ? -1 : 1;
        const int zFactor = zFlip ? -1 : 1;

        switch (drawPlane)
        {
            default:
            case xy:
                drawH = dimensions.x;
                drawW = dimensions.y;
                break;
            case zy:
                drawH = dimensions.z;
                drawW = dimensions.y;
                break;
            case zx:
                drawH = dimensions.z;
                drawW = dimensions.x;
                break;
        }

        if (autoScale)
        {
            float dimRatio = drawH / drawW;

            if (dimRatio >= height / width)
                scale = height / drawH;
            else
                scale = width / drawW;

            if (dimRatio >= height / width)
                scale = height / drawH;
            else
                scale = width / drawW;
        }
        drawW *= scale;
        drawH *= scale;

        juce::Rectangle<float> room (innerSpacing + 0.5f * (width - drawW),
                                     innerSpacing + 0.5f * (height - drawH),
                                     drawW,
                                     drawH);

        g.setColour (juce::Colours::white.withMultipliedSaturation (0.9f));
        g.setFont (10.0f);
        switch (drawPlane)
        {
            default:
            case xy:
                g.drawArrow (
                    juce::Line<float> (centreX, centreY, centreX, centreY - 20.0f * xFactor),
                    1.0f,
                    4.0f,
                    4.0f);
                g.drawArrow (
                    juce::Line<float> (centreX, centreY, centreX - 20.0f * yFactor, centreY),
                    1.0f,
                    4.0f,
                    4.0f);
                g.drawSingleLineText ("x", centreX + 2.0f, centreY + 2.0f - 9.0f * xFactor);
                g.drawSingleLineText ("y", centreX - 2.0f - 10.0f * yFactor, centreY + 7.0f);
                break;
            case zy:
                g.drawArrow (
                    juce::Line<float> (centreX, centreY, centreX, centreY - 20.0f * zFactor),
                    1.0f,
                    4.0f,
                    4.0f);
                g.drawArrow (
                    juce::Line<float> (centreX, centreY, centreX - 20.0f * yFactor, centreY),
                    1.0f,
                    4.0f,
                    4.0f);
                g.drawSingleLineText ("z", centreX + 2.0f, centreY + 2.0f - 9.0f * zFactor);
                g.drawSingleLineText ("y", centreX - 2.0f - 10.0f * yFactor, centreY + 7.0f);
                ;
                break;
            case zx:
                g.drawArrow (
                    juce::Line<float> (centreX, centreY, centreX, centreY - 20.0f * zFactor),
                    1.0f,
                    4.0f,
                    4.0f);
                g.drawArrow (
                    juce::Line<float> (centreX, centreY, centreX + 20.0f * xFactor, centreY),
                    1.0f,
                    4.0f,
                    4.0f);
                g.drawSingleLineText ("z", centreX + 2.0f, centreY + 2.0f - 9.0f * zFactor);
                g.drawSingleLineText ("x", centreX + 2.0f, centreY + 2.0f - 9.0f * xFactor);
                break;
        }

        g.setColour (juce::Colours::steelblue.withMultipliedAlpha (0.3f));
        g.fillRect (room);

        g.setColour (juce::Colours::white);
        g.drawRect (room, 1.0f);

        for (int i = elements.size(); --i >= 0;)
        {
            Element* handle = (Element*) elements.getUnchecked (i);

            juce::Vector3D<float> position = handle->getPosition();
            g.setColour (handle->isActive() ? handle->getColour() : juce::Colours::grey);

            juce::Path path;
            float posH, posW;
            switch (drawPlane)
            {
                default:
                case xy:
                    posH = position.x * xFactor;
                    posW = position.y * yFactor;
                    break;
                case zy:
                    posH = position.z * zFactor;
                    posW = position.y * yFactor;
                    break;
                case zx:
                    posH = position.z * zFactor;
                    posW = -position.x * xFactor;
                    break;
            }
            juce::Rectangle<float> temp (centreX - posW * scale - 10 / 2,
                                         centreY - posH * scale - 10 / 2,
                                         11,
                                         11);
            path.addEllipse (temp);
            g.fillPath (path);
        }
    };

    float setDimensions (juce::Vector3D<float> newDimensions)
    {
        dimensions = newDimensions;
        repaint();

        float width = getBounds().getWidth();
        float height = getBounds().getHeight();

        float drawH, drawW;
        float tempScale;
        switch (drawPlane)
        {
            default:
            case xy:
                drawH = dimensions.x;
                drawW = dimensions.y;
                break;
            case zy:
                drawH = dimensions.z;
                drawW = dimensions.y;
                break;
            case zx:
                drawH = dimensions.z;
                drawW = dimensions.x;
                break;
        }

        float dimRatio = drawH / drawW;

        if (dimRatio >= height / width)
            tempScale = height / drawH;
        else
            tempScale = width / drawW;

        return tempScale;
    }

    void useAutoScale (bool shouldUseAutoScale) { autoScale = shouldUseAutoScale; }
    bool usingAutoScale() { return autoScale; }

    void setScale (float newScale)
    {
        if (! autoScale)
            scale = newScale;
    }

    juce::Vector3D<float> getDimensions() { return dimensions; }

    void mouseDown (const juce::MouseEvent& event) override
    {
        Element* handle;

        auto bounds = getLocalBounds().toType<float>();
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();

        int nElem = elements.size();
        activeElem = -1;
        float activeDSquared = 80.0f; //dummy value
        if (nElem > 0)
        {
            const int xFactor = xFlip ? -1 : 1;
            const int yFactor = yFlip ? -1 : 1;
            const int zFactor = zFlip ? -1 : 1;

            auto pos = event.getPosition();

            float mouseX = (centreY - pos.getY());
            float mouseY = (centreX - pos.getX());

            if (drawPlane == zx)
                mouseY *= -1;

            for (int i = elements.size(); --i >= 0;)
            {
                handle = elements.getUnchecked (i);

                float posH, posW;
                juce::Vector3D<float> position = handle->getPosition();
                switch (drawPlane)
                {
                    default:
                    case xy:
                        posH = position.x * xFactor;
                        posW = position.y * yFactor;
                        break;
                    case zy:
                        posH = position.z * zFactor;
                        posW = position.y * yFactor;
                        break;
                    case zx:
                        posH = position.z * zFactor;
                        posW = position.x * xFactor;
                        break;
                }

                float tx = (mouseX - posH * scale);
                float ty = (mouseY - posW * scale);

                float dSquared = tx * tx + ty * ty;
                if (dSquared <= 80.0f && dSquared < activeDSquared)
                {
                    activeElem = i;
                    activeDSquared = dSquared;
                }
            }
        }
        if (activeElem != -1)
        {
            elements.getUnchecked (activeElem)->startMovement();
        }
    }

    void mouseDrag (const juce::MouseEvent& event) override
    {
        auto bounds = getLocalBounds().toType<float>();
        const auto centre = bounds.getCentre();
        const int xFactor = xFlip ? -1 : 1;
        const int yFactor = yFlip ? -1 : 1;
        const int zFactor = zFlip ? -1 : 1;

        if (activeElem != -1)
        {
            Element* handle = elements.getUnchecked (activeElem);
            handle->moveElement (event, centre, scale, drawPlane, this, xFactor, yFactor, zFactor);
            handle->repaintAllPlanesImIn();
            sendChanges (handle);
        }
        repaint();
    }

    void mouseUp (const juce::MouseEvent& event) override
    {
        if (activeElem != -1)
        {
            elements.getUnchecked (activeElem)->stopMovement();
        }
    }

    void setPlane (Planes PlaneToDraw) { drawPlane = PlaneToDraw; }

    void addListener (PositionPlaneListener* const listener)
    {
        jassert (listener != 0);
        if (listener != 0)
            listeners.add (listener);
    };
    void removeListener (PositionPlaneListener* const listener)
    {
        listeners.removeFirstMatchingValue (listener);
    };

    void sendChanges (Element* element)
    {
        for (int i = listeners.size(); --i >= 0;)
            ((PositionPlaneListener*) listeners.getUnchecked (i))
                ->PositionPlaneElementChanged (this, element);
    }

    void addElement (Element* const element)
    {
        jassert (element != 0);
        if (element != 0)
        {
            elements.add (element);
            element->addPlane (this);
        }
    };
    void removeElement (Element* const element)
    {
        element->removePlane (this);
        elements.removeFirstMatchingValue (element);
    };

    int indexofSmallestElement (float* array, int size)
    {
        int index = 0;

        for (int i = 1; i < size; i++)
        {
            if (array[i] < array[index])
                index = i;
        }

        return index;
    }

    void setXFlip (const bool flipped)
    {
        if (xFlip != flipped)
        {
            xFlip = flipped;
            repaint();
        }
    }

    void setYFlip (const bool flipped)
    {
        if (yFlip != flipped)
        {
            yFlip = flipped;
            repaint();
        }
    }

    void setZFlip (const bool flipped)
    {
        if (zFlip != flipped)
        {
            zFlip = flipped;
            repaint();
        }
    }

private:
    Planes drawPlane;
    juce::String suffix;

    bool xFlip = false;
    bool yFlip = false;
    bool zFlip = false;

    bool autoScale;
    juce::Vector3D<float> dimensions;
    float scale;
    int activeElem;
    bool activeElemWasUpBeforeDrag;
    juce::Array<void*> listeners;
    juce::Array<Element*> elements;
};
