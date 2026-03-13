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

class LaF : public juce::LookAndFeel_V4
{
public:
    const juce::Colour ClBackground = juce::Colour (0xFF2D2D2D);
    const juce::Colour ClFace = juce::Colour (0xFFD8D8D8);
    const juce::Colour ClFaceShadow = juce::Colour (0XFF272727);
    const juce::Colour ClFaceShadowOutline = juce::Colour (0xFF212121);
    const juce::Colour ClFaceShadowOutlineActive = juce::Colour (0xFF7C7C7C);
    const juce::Colour ClRotSliderArrow = juce::Colour (0xFF4A4A4A);
    const juce::Colour ClRotSliderArrowShadow = juce::Colour (0x445D5D5D);
    const juce::Colour ClSliderFace = juce::Colour (0xFF191919);
    const juce::Colour ClText = juce::Colour (0xFFFFFFFF);
    const juce::Colour ClTextTextboxbg = juce::Colour (0xFF000000);
    const juce::Colour ClSeperator = juce::Colour (0xFF979797);
    const juce::Colour ClWidgetColours[4] = { juce::Colour (0xFF00CAFF),
                                              juce::Colour (0xFF4FFF00),
                                              juce::Colour (0xFFFF9F00),
                                              juce::Colour (0xFFD0011B) };

    juce::Typeface::Ptr robotoLight, robotoRegular, robotoMedium, robotoBold;

    //float sliderThumbDiameter = 14.0f;
    float sliderBarSize = 8.0f;

    LaF()
    {
        robotoLight = juce::Typeface::createSystemTypefaceFor (
            BinaryData::RobotoLight_ttf,
            BinaryData::RobotoLight_ttfSize); //TODO: free this data
        robotoMedium = juce::Typeface::createSystemTypefaceFor (BinaryData::RobotoMedium_ttf,
                                                                BinaryData::RobotoMedium_ttfSize);
        robotoBold = juce::Typeface::createSystemTypefaceFor (BinaryData::RobotoBold_ttf,
                                                              BinaryData::RobotoBold_ttfSize);
        robotoRegular =
            juce::Typeface::createSystemTypefaceFor (BinaryData::RobotoRegular_ttf,
                                                     BinaryData::RobotoRegular_ttfSize); //

        setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::black);
        setColour (juce::Slider::thumbColourId, juce::Colour (0xCCFFFFFF));
        setColour (juce::TextButton::buttonColourId, juce::Colours::black);
        setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xFF2D2D2D));
        setColour (juce::ScrollBar::thumbColourId, juce::Colours::steelblue);
        setColour (juce::ScrollBar::thumbColourId, juce::Colours::steelblue);
        setColour (juce::PopupMenu::backgroundColourId,
                   juce::Colours::steelblue.withMultipliedAlpha (0.9f));
        setColour (juce::ListBox::backgroundColourId,
                   juce::Colours::steelblue.withMultipliedAlpha (0.1f));
        setColour (juce::ListBox::outlineColourId,
                   juce::Colours::steelblue.withMultipliedAlpha (0.3f));

        setColour (juce::TooltipWindow::backgroundColourId,
                   juce::Colours::steelblue.withMultipliedAlpha (0.9f));

        setColour (juce::TableHeaderComponent::backgroundColourId,
                   juce::Colours::lightgrey.withMultipliedAlpha (0.8f));
        setColour (juce::TableHeaderComponent::highlightColourId,
                   juce::Colours::steelblue.withMultipliedAlpha (0.3f));
    }

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& f) override
    {
        switch (f.getStyleFlags())
        {
            case 2:
                return robotoLight;
            case 1:
                return robotoBold;
            default:
                return robotoRegular;
        }
    }
    juce::Font getLabelFont (juce::Label& label) override
    {
        ignoreUnused (label);
        return juce::Font (robotoMedium);
    }

    juce::Font getPopupMenuFont() override
    {
        juce::Font font (robotoRegular);
        font.setHeight (14.0f);
        return font;
    }

    juce::Font getTextButtonFont (juce::TextButton& button, int height) override
    {
        ignoreUnused (button, height);
        juce::Font font (robotoMedium);
        font.setHeight (14.0f);
        return font;
    }

    juce::Font getAlertWindowMessageFont() override
    {
        juce::Font font (robotoRegular);
        font.setHeight (14.0f);
        return font;
    }

    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override
    {
        // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints

        int minXSpace = 0;
        int minYSpace = 0;

        juce::Slider::TextEntryBoxPosition textBoxPos = slider.getTextBoxPosition();

        if (textBoxPos == juce::Slider::TextBoxLeft || textBoxPos == juce::Slider::TextBoxRight)
            minXSpace = 30;
        else
            minYSpace = 15;

        if (slider.getSliderStyle() == juce::Slider::IncDecButtons)
            minXSpace = 18;

        juce::Rectangle<int> localBounds = slider.getLocalBounds();

        const int textBoxWidth =
            juce::jmax (0,
                        juce::jmin (slider.getTextBoxWidth(), localBounds.getWidth() - minXSpace));
        const int textBoxHeight = juce::jmax (
            0,
            juce::jmin (slider.getTextBoxHeight(), localBounds.getHeight() - minYSpace));

        juce::Slider::SliderLayout layout;

        // 2. set the textBox bounds

        if (textBoxPos != juce::Slider::NoTextBox)
        {
            if (slider.isBar())
            {
                layout.textBoxBounds = localBounds;
            }
            else
            {
                layout.textBoxBounds.setWidth (textBoxWidth);
                layout.textBoxBounds.setHeight (textBoxHeight);

                if (textBoxPos == juce::Slider::TextBoxLeft)
                    layout.textBoxBounds.setX (0);
                else if (textBoxPos == juce::Slider::TextBoxRight)
                    layout.textBoxBounds.setX (localBounds.getWidth() - textBoxWidth);
                else /* above or below -> centre horizontally */
                    layout.textBoxBounds.setX ((localBounds.getWidth() - textBoxWidth) / 2);

                if (textBoxPos == juce::Slider::TextBoxAbove)
                    layout.textBoxBounds.setY (0);
                else if (textBoxPos == juce::Slider::TextBoxBelow)
                    layout.textBoxBounds.setY (localBounds.getHeight() - textBoxHeight);
                else /* left or right -> centre vertically */
                    layout.textBoxBounds.setY ((localBounds.getHeight() - textBoxHeight) / 2);
            }
        }

        // 3. set the slider bounds

        layout.sliderBounds = localBounds;

        if (slider.isBar())
        {
            layout.sliderBounds.reduce (1, 1); // bar border
        }
        else
        {
            if (textBoxPos == juce::Slider::TextBoxLeft)
                layout.sliderBounds.removeFromLeft (textBoxWidth);
            else if (textBoxPos == juce::Slider::TextBoxRight)
                layout.sliderBounds.removeFromRight (textBoxWidth);
            else if (textBoxPos == juce::Slider::TextBoxAbove)
                layout.sliderBounds.removeFromTop (textBoxHeight);
            else if (textBoxPos == juce::Slider::TextBoxBelow)
                layout.sliderBounds.removeFromBottom (textBoxHeight);

            const int thumbIndent = getSliderThumbRadius (slider);

            if (slider.isHorizontal())
                layout.sliderBounds.reduce (thumbIndent, 0);
            else if (slider.isVertical())
                layout.sliderBounds.reduce (0, thumbIndent);
        }

        return layout;
    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        float alpha = label.isEnabled() ? 1.0f : 0.4f;
        g.fillAll (label.findColour (juce::Label::backgroundColourId));
        juce::Rectangle<int> bounds = label.getLocalBounds();
        float x = (float) bounds.getX();
        float y = (float) bounds.getY();
        float w = (float) bounds.getWidth();
        float h = (float) bounds.getHeight();
        juce::Path p;
        p.addRoundedRectangle (x, y, w, h, h / 2.0f);
        g.setColour (ClTextTextboxbg.withMultipliedAlpha (alpha));
        g.fillPath (p);

        if (! label.isBeingEdited())
        {
            const float fontAlpha = label.isEnabled() ? 1.0f : 0.5f;
            const juce::Font font (robotoLight);

            //g.setColour (ClText.withMultipliedAlpha (alpha));
            g.setColour (ClText.withMultipliedAlpha (fontAlpha));
            g.setFont (robotoMedium);
            g.setFont (13.f);

            juce::Rectangle<int> textArea (
                label.getBorderSize().subtractedFrom (label.getLocalBounds()));

            g.drawFittedText (label.getText(),
                              textArea,
                              label.getJustificationType(),
                              juce::jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                              label.getMinimumHorizontalScale());

            g.setColour (
                label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (fontAlpha));
        }
        else if (label.isEnabled())
        {
            g.setColour (label.findColour (juce::Label::outlineColourId));
        }
    }

    void drawCornerResizer (juce::Graphics& g,
                            int w,
                            int h,
                            bool /*isMouseOver*/,
                            bool /*isMouseDragging*/) override
    {
        g.setColour (juce::Colours::white.withMultipliedAlpha (0.5f));

        juce::Path triangle;
        triangle.startNewSubPath (w, h);
        triangle.lineTo (0.5f * w, h);
        triangle.lineTo (w, 0.5f * h);
        triangle.closeSubPath();

        g.fillPath (triangle);
    }

    void fillTextEditorBackground (juce::Graphics& g,
                                   int width,
                                   int height,
                                   juce::TextEditor& textEditor) override
    {
        if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) != nullptr)
        {
            g.setColour (textEditor.findColour (juce::TextEditor::backgroundColourId));
            g.fillRect (0, 0, width, height);

            g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
            g.drawHorizontalLine (height - 1, 0.0f, static_cast<float> (width));
        }
        else
        {
            juce::Path p;
            p.addRoundedRectangle (0, 0, width, height, 12.0f);
            //g.setColour (ClTextTextboxbg);
            g.setColour (textEditor.findColour (juce::TextEditor::backgroundColourId));
            g.fillPath (p);
        }
    }

    void drawTextEditorOutline (juce::Graphics& g,
                                int width,
                                int height,
                                juce::TextEditor& textEditor) override
    {
        if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) == nullptr)
        {
            if (textEditor.isEnabled())
            {
                if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
                {
                    g.setColour (juce::Colours::white.withMultipliedAlpha (0.8f));
                    g.drawRoundedRectangle (0.5,
                                            0.5,
                                            width - 1,
                                            height - 1,
                                            (height - 1) / 2.0f,
                                            0.8f);
                }
                else
                {
                    g.setColour (juce::Colours::white.withMultipliedAlpha (0.8f));
                    g.drawRoundedRectangle (0, 0, width, height, height / 2.0f, 0);
                }
            }
        }
    }

    void drawTableHeaderBackground (juce::Graphics& g, juce::TableHeaderComponent& header) override
    {
        juce::Rectangle<int> r (header.getLocalBounds());
        auto outlineColour = header.findColour (juce::TableHeaderComponent::outlineColourId);

        g.setColour (outlineColour);
        g.fillRect (r.removeFromBottom (1));

        g.setColour (header.findColour (juce::TableHeaderComponent::backgroundColourId));
        g.fillRect (r);

        g.setColour (outlineColour);

        for (int i = header.getNumColumns (true); --i >= 0;)
            g.fillRect (header.getColumnPosition (i).removeFromRight (1));
    }

    void drawTableHeaderColumn (juce::Graphics& g,
                                juce::TableHeaderComponent& header,
                                const juce::String& columnName,
                                int /*columnId*/,
                                int width,
                                int height,
                                bool isMouseOver,
                                bool isMouseDown,
                                int columnFlags) override
    {
        auto highlightColour = header.findColour (juce::TableHeaderComponent::highlightColourId);

        if (isMouseDown)
            g.fillAll (highlightColour);
        else if (isMouseOver)
            g.fillAll (highlightColour.withMultipliedAlpha (0.625f));

        juce::Rectangle<int> area (width, height);
        area.reduce (4, 0);

        if ((columnFlags
             & (juce::TableHeaderComponent::sortedForwards
                | juce::TableHeaderComponent::sortedBackwards))
            != 0)
        {
            juce::Path sortArrow;
            sortArrow.addTriangle (
                0.0f,
                0.0f,
                0.5f,
                (columnFlags & juce::TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
                1.0f,
                0.0f);

            g.setColour (juce::Colour (0x99000000));
            g.fillPath (sortArrow,
                        sortArrow.getTransformToScaleToFit (
                            area.removeFromRight (height / 2).reduced (2).toFloat(),
                            true));
        }

        g.setColour (header.findColour (juce::TableHeaderComponent::textColourId));
        g.setFont (robotoRegular);
        g.setFont (height * 0.6f);
        g.drawFittedText (columnName, area, juce::Justification::centred, 1);
    }

    void drawLinearSlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle style,
                           juce::Slider& slider) override
    {
        //g.fillAll (slider.findColour (juce::Slider::backgroundColourId));

        //Label* l = createSliderTextBox(slider);
        //l->showEditor();

        if (style == juce::Slider::LinearBar || style == juce::Slider::LinearBarVertical)
        {
            const float fx = (float) x, fy = (float) y, fw = (float) width, fh = (float) height;

            juce::Path p;

            if (style == juce::Slider::LinearBarVertical)
                p.addRectangle (fx, sliderPos, fw, 1.0f + fh - sliderPos);
            else
                p.addRectangle (fx, fy, sliderPos - fx, fh);

            juce::Colour baseColour (
                slider.findColour (juce::Slider::rotarySliderFillColourId)
                    .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                    .withMultipliedAlpha (1.0f));

            g.setColour (baseColour);
            g.fillPath (p);

            const float lineThickness =
                juce::jmin (15.0f, juce::jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
        {
            drawLinearSliderBackground (g,
                                        x,
                                        y,
                                        width,
                                        height,
                                        sliderPos,
                                        minSliderPos,
                                        maxSliderPos,
                                        style,
                                        slider);
            drawLinearSliderThumb (g,
                                   x,
                                   y,
                                   width,
                                   height,
                                   sliderPos,
                                   minSliderPos,
                                   maxSliderPos,
                                   style,
                                   slider);
        }
    }

    void drawLinearSliderBackground (juce::Graphics& g,
                                     int x,
                                     int y,
                                     int width,
                                     int height,
                                     float sliderPos,
                                     float minSliderPos,
                                     float maxSliderPos,
                                     const juce::Slider::SliderStyle style,
                                     juce::Slider& slider) override
    {
        const float sliderRadius = 8.f; //getSliderThumbRadius (slider) - 5.0f;
        juce::Path slbg;
        juce::Path clbar;

        juce::Colour statusColour = slider.findColour (juce::Slider::rotarySliderOutlineColourId)
                                        .withMultipliedAlpha (0.3f);

        const float min = static_cast<float> (slider.getMinimum());
        const float max = static_cast<float> (slider.getMaximum());
        const float zeroPos = -min / (max - min);
        bool isTwoValue = (style == juce::Slider::SliderStyle::TwoValueVertical
                           || style == juce::Slider::SliderStyle::TwoValueHorizontal);

        if (slider.isHorizontal())
        {
            const float iy = y + height * 0.5f - sliderRadius * 0.5f;
            juce::Rectangle<float> r (x - sliderRadius * 0.5f,
                                      iy,
                                      width + sliderRadius,
                                      sliderRadius);
            slbg.addRoundedRectangle (r, sliderRadius / 2.0, sliderRadius / 2.0);

            if (isTwoValue)
                clbar.addRoundedRectangle (
                    juce::Rectangle<float> (juce::Point<float> (minSliderPos, iy),
                                            juce::Point<float> (maxSliderPos, iy + sliderRadius)),
                    sliderRadius / 2.0,
                    sliderRadius / 2.0);
            else
                clbar.addRoundedRectangle (
                    juce::Rectangle<float> (juce::Point<float> (x + width * zeroPos, iy),
                                            juce::Point<float> (sliderPos, iy + sliderRadius)),
                    sliderRadius / 2.0,
                    sliderRadius / 2.0);
        }
        else
        {
            const float ix = x + width * 0.5f - sliderRadius * 0.5f;
            juce::Rectangle<float> r (ix,
                                      y - sliderRadius * 0.5f,
                                      sliderRadius,
                                      height + sliderRadius);
            slbg.addRoundedRectangle (r, sliderRadius / 2.0, sliderRadius / 2.0);
            clbar.addRoundedRectangle (
                juce::Rectangle<float> (
                    juce::Point<float> (ix + 1.0f, y + height * (1.0f - zeroPos)),
                    juce::Point<float> (ix - 1.0f + sliderRadius, sliderPos)),
                sliderRadius / 2.0,
                sliderRadius / 2.0);
        }

        g.setColour (ClSliderFace);
        g.fillPath (slbg);
        g.setColour (statusColour);
        g.fillPath (clbar);
        g.setColour (ClFaceShadowOutline);

        g.strokePath (slbg, juce::PathStrokeType (1.0f));
    }

    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        drawRotarySliderDual (g,
                              x,
                              y,
                              width,
                              height,
                              sliderPos,
                              rotaryStartAngle,
                              rotaryEndAngle,
                              slider,
                              false);
    }

    void drawRotarySliderDual (juce::Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPos,
                               float rotaryStartAngle,
                               float rotaryEndAngle,
                               juce::Slider& slider,
                               bool isDual)
    {
        bool isEnabled = slider.isEnabled();
        const float alpha = isEnabled ? 1.0f : 0.4f;
        const float radius = juce::jmin (width / 2, height / 2);
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;

        const float min = static_cast<float> (slider.getMinimum());
        const float max = static_cast<float> (slider.getMaximum());
        const float zeroPos = -min / (max - min);
        const float zeroAngle = rotaryStartAngle + zeroPos * (rotaryEndAngle - rotaryStartAngle);
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const float negAngle =
            rotaryStartAngle + (2 * zeroPos - sliderPos) * (rotaryEndAngle - rotaryStartAngle);

        const float bedThickness = 2.0f;
        const float bedOutline = 1.4f;
        const float statusOutline = 1.6f;
        const float extraMargin = 1.0f;

        const float pointerThickness = 1.2f;
        const float pointerLength =
            (radius - extraMargin - statusOutline - bedOutline - bedThickness - 1.0f) * 0.8f;

        juce::Path p, q, a;
        juce::Rectangle<float> r = juce::Rectangle<float> (rx, ry, rw, rw);

        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        const juce::Colour statusColour =
            slider.findColour (juce::Slider::rotarySliderOutlineColourId);
        //status ring
        g.setColour (statusColour.withMultipliedAlpha (alpha));

        a.addCentredArc (centreX,
                         centreY,
                         radius - extraMargin,
                         radius - extraMargin,
                         0.0f,
                         zeroAngle,
                         angle,
                         true);
        if (isDual)
            a.addCentredArc (centreX,
                             centreY,
                             radius - extraMargin,
                             radius - extraMargin,
                             0.0f,
                             negAngle,
                             zeroAngle,
                             true);
        g.strokePath (a, juce::PathStrokeType (statusOutline));

        //bed ellipse
        g.setColour (ClFaceShadow);
        g.fillEllipse (r.reduced (extraMargin + statusOutline));

        //(isMouseOver)?g.setColour(ClFaceShadowOutlineActive) : g.setColour (ClFaceShadowOutline);
        (isMouseOver) ? g.setColour (statusColour.withMultipliedAlpha (0.4f))
                      : g.setColour (ClFaceShadowOutline);
        g.drawEllipse (r.reduced (extraMargin + statusOutline), bedOutline);

        //knob
        g.setColour (ClFace.withMultipliedAlpha (alpha));
        g.fillEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness));
        g.setColour (statusColour.withMultipliedAlpha (alpha));
        g.drawEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness),
                       statusOutline);

        g.setColour (ClRotSliderArrowShadow.withMultipliedAlpha (alpha));
        g.drawEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness + 1.0f),
                       1.0f);

        q.addRectangle (pointerThickness * 0.3f, -radius + 6.0f, pointerThickness, pointerLength);
        q.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (ClRotSliderArrowShadow.withMultipliedAlpha (alpha));
        g.fillPath (q);

        p.addRectangle (-pointerThickness * 0.5f, -radius + 6.0f, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (ClRotSliderArrow.withMultipliedAlpha (alpha));
        g.fillPath (p);
    }

    void drawLinearSliderThumb (juce::Graphics& g,
                                int x,
                                int y,
                                int width,
                                int height,
                                float sliderPos,
                                float minSliderPos,
                                float maxSliderPos,
                                const juce::Slider::SliderStyle style,
                                juce::Slider& slider) override
    {
        const float sliderRadius = 7.0f;

        juce::Colour knobColour = slider.findColour (juce::Slider::rotarySliderOutlineColourId)
                                      .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);
        const float outlineThickness = slider.isEnabled() ? 1.9f : 0.3f;

        if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearVertical)
        {
            float kx, ky;

            if (style == juce::Slider::LinearVertical)
            {
                kx = x + width * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = y + height * 0.5f;
            }

            drawRoundThumb (g, kx, ky, sliderRadius * 2.0f, knobColour, outlineThickness);
        }
        else if (style == juce::Slider::TwoValueVertical)
        {
            drawRoundThumb (g,
                            juce::jmax (sliderRadius, x + width * 0.5f),
                            minSliderPos,
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);

            drawRoundThumb (g,
                            juce::jmax (sliderRadius, x + width * 0.5f),
                            maxSliderPos,
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);
        }
        else if (style == juce::Slider::TwoValueHorizontal)
        {
            drawRoundThumb (g,
                            minSliderPos,
                            juce::jmax (sliderRadius, y + height * 0.5f),
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);

            drawRoundThumb (g,
                            maxSliderPos,
                            juce::jmax (sliderRadius, y + height * 0.5f),
                            sliderRadius * 2.0f,
                            knobColour,
                            outlineThickness);
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g,
                                                   x,
                                                   y,
                                                   width,
                                                   height,
                                                   sliderPos,
                                                   minSliderPos,
                                                   maxSliderPos,
                                                   style,
                                                   slider);
        }
    }
    void drawRoundThumb (juce::Graphics& g,
                         const float centreX,
                         const float centreY,
                         const float diameter,
                         const juce::Colour& colour,
                         float outlineThickness)
    {
        //        const juce::Rectangle<float> a (x, y, diameter, diameter);

        const float newDiameter = (diameter - outlineThickness);
        const float halfThickness = newDiameter * 0.5f;

        juce::Path p;
        p.addEllipse (centreX - halfThickness, centreY - halfThickness, newDiameter, newDiameter);

        g.setColour (ClFace);
        g.fillPath (p);

        g.setColour (colour);
        g.strokePath (p, juce::PathStrokeType (outlineThickness));

        g.setColour (ClRotSliderArrowShadow);
        g.drawEllipse (centreX + 1.0f - halfThickness,
                       centreY + 1.0f - halfThickness,
                       diameter - outlineThickness - 1.0f,
                       diameter - outlineThickness - 1.0f,
                       1.4f);
    }

    juce::Button* createSliderButton (juce::Slider&, const bool isIncrement) override
    {
        return new juce::TextButton (isIncrement ? "+" : "-", juce::String());
        //return new ArrowButton (juce::String(),isIncrement ? 0.75 : 0.25f,juce::Colours::white);
    }

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        juce::Rectangle<float> buttonArea (0.0f, 0.0f, button.getWidth(), button.getHeight());
        buttonArea.reduce (1.0f, 1.0f);

        g.setColour (backgroundColour);
        if (isButtonDown)
            buttonArea.reduce (0.8f, 0.8f);
        else if (isMouseOverButton)
            buttonArea.reduce (0.4f, 0.4f);

        g.drawRoundedRectangle (buttonArea, 2.0f, 1.0f);

        buttonArea.reduce (1.5f, 1.5f);
        g.setColour (backgroundColour.withMultipliedAlpha (isButtonDown        ? 1.0f
                                                           : isMouseOverButton ? 0.5f
                                                                               : 0.2f));

        g.fillRoundedRectangle (buttonArea, 2.0f);
    }

    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool /*isMouseOverButton*/,
                         bool /*isButtonDown*/) override
    {
        juce::Font font (getTextButtonFont (button, button.getHeight()));
        g.setFont (font);
        g.setColour (button
                         .findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                              : juce::TextButton::textColourOffId)
                         .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

        const int yIndent = juce::jmin (4, button.proportionOfHeight (0.3f));
        const int cornerSize = juce::jmin (button.getHeight(), button.getWidth()) / 2;

        const int fontHeight = juce::roundToInt (font.getHeight() * 0.6f);
        const int leftIndent =
            juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
        const int rightIndent =
            juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
        const int textWidth = button.getWidth() - leftIndent - rightIndent;

        if (textWidth > 0)
            g.drawFittedText (button.getButtonText(),
                              leftIndent,
                              yIndent,
                              textWidth,
                              button.getHeight() - yIndent * 2,
                              juce::Justification::centred,
                              2);
    }

    void drawToggleButton (juce::Graphics& g,
                           juce::ToggleButton& button,
                           bool isMouseOverButton,
                           bool isButtonDown) override
    {
        if (button.getButtonText() == "ON/OFF")
        {
            juce::Colour baseColour (
                juce::Colours::black
                    .withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                    .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

            const float width = button.getWidth();
            const float height = button.getHeight();
            bool isOn = button.getToggleState();
            const float cornerSize = juce::jmin (15.0f, juce::jmin (width, height) * 0.45f);

            juce::Path outline;
            outline.addRoundedRectangle (0.5f, 0.5f, width - 1, height - 1, cornerSize, cornerSize);

            g.setColour (baseColour);
            g.fillPath (outline);
            if (isMouseOverButton)
            {
                g.setColour (button.findColour (juce::ToggleButton::tickColourId)
                                 .withMultipliedAlpha (isButtonDown ? 0.8f : 0.4f));
                g.strokePath (outline, juce::PathStrokeType (isButtonDown ? 1.0f : 0.8f));
            }
            g.setFont (robotoMedium);
            g.setFont (height - 1);
            g.setColour (isOn ? button.findColour (juce::ToggleButton::tickColourId)
                              : juce::Colours::white);
            g.drawText (isOn ? "ON" : "OFF",
                        0,
                        0,
                        static_cast<int> (width),
                        static_cast<int> (height),
                        juce::Justification::centred);
        }

        else
        {
            const auto fontSize = juce::jmin (15.0f, button.getHeight() * 0.75f);
            const auto tickWidth = fontSize * 1.1f;

            drawTickBox (g,
                         button,
                         4.0f,
                         (button.getHeight() - tickWidth) * 0.5f,
                         tickWidth,
                         tickWidth,
                         button.getToggleState(),
                         button.isEnabled(),
                         isMouseOverButton,
                         isButtonDown);

            g.setColour (button.findColour (juce::ToggleButton::textColourId));
            g.setFont (fontSize);

            if (! button.isEnabled())
                g.setOpacity (0.5f);

            g.setFont (robotoMedium);
            g.drawFittedText (button.getButtonText(),
                              button.getLocalBounds()
                                  .withTrimmedLeft (juce::roundToInt (tickWidth) + 10)
                                  .withTrimmedRight (2),
                              juce::Justification::centredLeft,
                              10);
        }
    }

    void drawTickBox (juce::Graphics& g,
                      juce::Component& component,
                      float x,
                      float y,
                      float w,
                      float h,
                      bool ticked,
                      bool isEnabled,
                      bool isMouseOverButton,
                      bool isButtonDown) override
    {
        juce::ignoreUnused (isEnabled);

        const float boxSize = w * 0.8f;

        juce::Rectangle<float> buttonArea (x + (w - boxSize) * 0.5f,
                                           y + (h - boxSize) * 0.5f,
                                           boxSize,
                                           boxSize);

        g.setColour (component.findColour (juce::ToggleButton::tickColourId)
                         .withMultipliedAlpha (ticked              ? 1.0f
                                               : isMouseOverButton ? 0.7f
                                                                   : 0.5f));

        if (isButtonDown)
            buttonArea.reduce (0.8f, 0.8f);
        else if (isMouseOverButton)
            buttonArea.reduce (0.4f, 0.4f);

        g.drawRoundedRectangle (buttonArea, 2.0f, 1.0f);

        buttonArea.reduce (1.5f, 1.5f);
        g.setColour (component.findColour (juce::ToggleButton::tickColourId)
                         .withMultipliedAlpha (ticked              ? 1.0f
                                               : isMouseOverButton ? 0.5f
                                                                   : 0.2f));

        g.fillRoundedRectangle (buttonArea, 2.0f);
    }

    juce::Path getTickShape (const float height) override
    {
        juce::Path p;
        juce::Path stroke;
        stroke.addRoundedRectangle (juce::Rectangle<float> (-1.0f, -5.0f, 2.0f, 10.0f), 0.1f, 0.1f);
        p.addPath (stroke,
                   juce::AffineTransform().rotation (0.25f * juce::MathConstants<float>::pi));
        p.addPath (stroke,
                   juce::AffineTransform().rotation (-0.25f * juce::MathConstants<float>::pi));
        p.scaleToFit (0, 0, height * 2.0f, height, true);
        return p;
    }

    void drawGroupComponentOutline (juce::Graphics& g,
                                    int width,
                                    int height,
                                    const juce::String& text,
                                    const juce::Justification& position,
                                    juce::GroupComponent& group) override
    {
        ignoreUnused (height, group);

        juce::Rectangle<int> r (6, 0, width - 6, 15);
        g.setColour (ClText);
        g.setFont (robotoMedium);
        g.setFont (18.0f);
        g.drawFittedText (text, r, position, 1, 0.f);

        g.setColour (ClSeperator);
        g.drawLine (0, 18, width, 18, .8f);
    }
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (0, 0, box.getWidth() - box.getHeight(), box.getHeight());

        label.setFont (getLabelFont (label));
    }

    void drawComboBox (juce::Graphics& g,
                       int width,
                       int height,
                       bool isButtonDown,
                       int buttonX,
                       int buttonY,
                       int buttonW,
                       int buttonH,
                       juce::ComboBox& box) override
    {
        juce::ignoreUnused (width, height, isButtonDown);

        juce::Rectangle<int> buttonArea (buttonX, buttonY, buttonW, buttonH);
        juce::Path path;
        path.startNewSubPath (buttonX + 3.0f, buttonArea.getCentreY() - 2.0f);
        path.lineTo (buttonArea.getCentreX(), buttonArea.getCentreY() + 3.0f);
        path.lineTo (buttonArea.getRight() - 3.0f, buttonArea.getCentreY() - 2.0f);

        g.setColour (juce::Colours::white.withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
        g.strokePath (path, juce::PathStrokeType (2.0f));
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g,
                                     const juce::Rectangle<int>& area,
                                     const juce::String& sectionName) override
    {
        g.setFont (robotoBold);
        g.setFont (18.0f);
        g.setColour (findColour (juce::PopupMenu::headerTextColourId));

        g.drawFittedText (sectionName,
                          area.getX() + 12,
                          area.getY(),
                          area.getWidth() - 16,
                          (int) (area.getHeight() * 0.8f),
                          juce::Justification::bottomLeft,
                          1);
    }

    void drawPopupMenuItem (juce::Graphics& g,
                            const juce::Rectangle<int>& area,
                            const bool isSeparator,
                            const bool isActive,
                            const bool isHighlighted,
                            const bool isTicked,
                            const bool hasSubMenu,
                            const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon,
                            const juce::Colour* const textColourToUse) override
    {
        if (isSeparator)
        {
            juce::Rectangle<int> r (area.reduced (5, 0));
            r.removeFromTop (r.getHeight() / 2 - 1);

            g.setColour (juce::Colour (0x33000000));
            g.fillRect (r.removeFromTop (1));

            g.setColour (juce::Colour (0x66ffffff));
            g.fillRect (r.removeFromTop (1));
        }
        else
        {
            juce::Colour textColour (findColour (juce::PopupMenu::textColourId));

            if (textColourToUse != nullptr)
                textColour = *textColourToUse;

            juce::Rectangle<int> r (area.reduced (1));

            if (isHighlighted)
            {
                g.setColour (findColour (juce::PopupMenu::highlightedBackgroundColourId));
                g.fillRect (r);

                g.setColour (findColour (juce::PopupMenu::highlightedTextColourId));
            }
            else
            {
                g.setColour (textColour);
            }

            if (! isActive)
                g.setOpacity (0.3f);

            juce::Font font (getPopupMenuFont());

            const float maxFontHeight = area.getHeight() / 1.3f;

            if (font.getHeight() > maxFontHeight)
                font.setHeight (maxFontHeight);

            g.setFont (font);

            juce::Rectangle<float> iconArea (
                r.removeFromLeft ((r.getHeight() * 5) / 4).reduced (3).toFloat());

            if (icon != nullptr)
            {
                icon->drawWithin (g,
                                  iconArea,
                                  juce::RectanglePlacement::centred
                                      | juce::RectanglePlacement::onlyReduceInSize,
                                  1.0f);
            }
            else if (isTicked)
            {
                const juce::Path tick (getTickShape (1.0f));
                g.fillPath (tick, tick.getTransformToScaleToFit (iconArea, true));
            }

            if (hasSubMenu)
            {
                const float arrowH = 0.6f * getPopupMenuFont().getAscent();

                const float x = (float) r.removeFromRight ((int) arrowH).getX();
                const float halfH = (float) r.getCentreY();

                juce::Path p;
                p.addTriangle (x,
                               halfH - arrowH * 0.5f,
                               x,
                               halfH + arrowH * 0.5f,
                               x + arrowH * 0.6f,
                               halfH);

                g.fillPath (p);
            }

            r.removeFromRight (3);
            g.drawFittedText (text, r, juce::Justification::centredLeft, 1);

            if (shortcutKeyText.isNotEmpty())
            {
                juce::Font f2 (font);
                f2.setHeight (f2.getHeight() * 0.75f);
                f2.setHorizontalScale (0.95f);
                g.setFont (f2);

                g.drawText (shortcutKeyText, r, juce::Justification::centredRight, true);
            }
        }
    }

    void drawCallOutBoxBackground (juce::CallOutBox& box,
                                   juce::Graphics& g,
                                   const juce::Path& path,
                                   juce::Image& cachedImage) override
    {
        if (cachedImage.isNull())
        {
            cachedImage = { juce::Image::ARGB, box.getWidth(), box.getHeight(), true };
            juce::Graphics g2 (cachedImage);

            juce::DropShadow (juce::Colours::black.withAlpha (0.7f), 8, { 0, 2 })
                .drawForPath (g2, path);
        }

        g.setColour (juce::Colours::black);
        g.drawImageAt (cachedImage, 0, 0);

        g.setColour (ClBackground.withAlpha (0.8f));
        g.fillPath (path);

        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.strokePath (path, juce::PathStrokeType (1.0f));
    }
};
