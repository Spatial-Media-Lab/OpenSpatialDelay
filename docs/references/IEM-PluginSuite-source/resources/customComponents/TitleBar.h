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

#include "../ambisonicTools.h"
#include "TitleBarPaths.h"

#ifdef JUCE_OSC_H_INCLUDED
    #include "../OSC/OSCStatus.h"
#endif

class AlertSymbol : public juce::Component, public juce::TooltipClient
{
public:
    AlertSymbol() : juce::Component()
    {
        warningSign.loadPathFromData (WarningSignData, sizeof (WarningSignData));
        setBufferedToImage (true);
    }

    juce::String getTooltip() override
    {
        return "Not enough channels available \n for your current setting.";
    }

    void paint (juce::Graphics& g) override
    {
        warningSign.applyTransform (
            warningSign.getTransformToScaleToFit (getLocalBounds().toFloat(),
                                                  true,
                                                  juce::Justification::centred));
        g.setColour (juce::Colours::yellow);
        g.fillPath (warningSign);
    }

private:
    juce::Path warningSign;
};

class IOWidget : public juce::Component
{
public:
    IOWidget() : juce::Component()
    {
        addChildComponent (alert);
        alert.setBounds (15, 15, 15, 15);
    }

    virtual const int getComponentSize() = 0;
    virtual void setMaxSize (int maxSize) = 0;

    void setBusTooSmall (bool isBusTooSmall)
    {
        busTooSmall = isBusTooSmall;
        alert.setVisible (isBusTooSmall);
    }

    bool isBusTooSmall() { return busTooSmall; }

private:
    AlertSymbol alert;
    bool busTooSmall = false;
};

class NoIOWidget : public IOWidget
{
public:
    NoIOWidget() {}
    void setMaxSize (int maxSize) override { juce::ignoreUnused (maxSize); }
    const int getComponentSize() override { return 0; }
};

class BinauralIOWidget : public IOWidget
{
public:
    BinauralIOWidget() : IOWidget()
    {
        BinauralPath.loadPathFromData (BinauralPathData, sizeof (BinauralPathData));
        setBufferedToImage (true);
    }

    const int getComponentSize() override { return 30; }
    void setMaxSize (int maxSize) override { juce::ignoreUnused (maxSize); }
    void paint (juce::Graphics& g) override
    {
        BinauralPath.applyTransform (
            BinauralPath
                .getTransformToScaleToFit (0, 0, 30, 30, true, juce::Justification::centred));
        g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (BinauralPath);
    }

private:
    juce::Path BinauralPath;
};

template <int maxChannels, bool selectable = true>
class AudioChannelsIOWidget : public IOWidget, private juce::ComboBox::Listener
{
public:
    AudioChannelsIOWidget() : IOWidget()
    {
        WaveformPath.loadPathFromData (WaveformPathData, sizeof (WaveformPathData));
        setBufferedToImage (true);

        if (selectable)
        {
            cbChannels.reset (new juce::ComboBox());
            addAndMakeVisible (cbChannels.get());
            cbChannels->setJustificationType (juce::Justification::centred);
            cbChannels->addSectionHeading ("Number of channels");
            cbChannels->addItem ("Auto", 1);
            for (int i = 1; i <= maxChannels; ++i)
                cbChannels->addItem (juce::String (i), i + 1);
            cbChannels->setBounds (35, 8, 70, 15);
            cbChannels->addListener (this);
        }
    }

    const int getComponentSize() override { return selectable ? 110 : 75; }

    void updateDisplayTextIfNotSelectable()
    {
        if (availableChannels < channelSizeIfNotSelectable)
        {
            displayTextIfNotSelectable =
                juce::String (channelSizeIfNotSelectable) + " (bus too small)";
            setBusTooSmall (true);
        }
        else
        {
            displayTextIfNotSelectable = juce::String (channelSizeIfNotSelectable);
            setBusTooSmall (false);
        }
        repaint();
    }

    void checkIfBusIsTooSmall()
    {
        if (availableChannels < cbChannels->getSelectedId() - 1)
            setBusTooSmall (true);
        else
            setBusTooSmall (false);
    }

    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override
    {
        juce::ignoreUnused (comboBoxThatHasChanged);
        checkIfBusIsTooSmall();
    }

    void setMaxSize (int maxPossibleNumberOfChannels) override
    {
        if (availableChannels != maxPossibleNumberOfChannels)
        {
            availableChannels = maxPossibleNumberOfChannels;
            if (selectable)
            {
                if (maxPossibleNumberOfChannels > 0)
                    cbChannels->changeItemText (
                        1,
                        "Auto (" + juce::String (maxPossibleNumberOfChannels) + ")");
                else
                    cbChannels->changeItemText (1, "(Auto)");
                int currId = cbChannels->getSelectedId();
                if (currId == 0)
                    currId = 1; //bad work around
                int i;
                for (i = 1; i <= maxPossibleNumberOfChannels; ++i)
                {
                    cbChannels->changeItemText (i + 1, juce::String (i));
                }
                for (i = maxPossibleNumberOfChannels + 1; i <= maxChannels; ++i)
                {
                    cbChannels->changeItemText (i + 1, juce::String (i) + " (bus too small)");
                }
                checkIfBusIsTooSmall();

                cbChannels->setText (cbChannels->getItemText (cbChannels->indexOfItemId (currId)));
            }
            else
            {
                updateDisplayTextIfNotSelectable();
            }
        }
    }

    void setSizeIfUnselectable (int newSize)
    {
        if (! selectable && channelSizeIfNotSelectable != newSize)
        {
            channelSizeIfNotSelectable = newSize;
            updateDisplayTextIfNotSelectable();
        }
    }

    juce::ComboBox* getChannelsCbPointer()
    {
        if (selectable)
            return cbChannels.get();
        return nullptr;
    }

    void paint (juce::Graphics& g) override
    {
        WaveformPath.applyTransform (
            WaveformPath
                .getTransformToScaleToFit (0, 0, 30, 30, true, juce::Justification::centred));
        g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (WaveformPath);

        if (! selectable)
        {
            g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
            g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 1)));
            g.setFont (15.0f);
            g.drawFittedText (displayTextIfNotSelectable,
                              35,
                              0,
                              40,
                              30,
                              juce::Justification::centredLeft,
                              2);
        }
    }

private:
    std::unique_ptr<juce::ComboBox> cbChannels;
    juce::Path WaveformPath;
    int availableChannels { 64 };
    int channelSizeIfNotSelectable = maxChannels;
    juce::String displayTextIfNotSelectable = juce::String (maxChannels);
};

template <int order = 7, bool selectable = true>
class AmbisonicIOWidget : public IOWidget
{
public:
    AmbisonicIOWidget() : IOWidget()
    {
        AmbiLogoPath.loadPathFromData (AmbiLogoPathData, sizeof (AmbiLogoPathData));
        setBufferedToImage (true);

        if (selectable)
        {
            addAndMakeVisible (&cbOrder);
            cbOrder.setJustificationType (juce::Justification::centred);
            cbOrder.setBounds (35, 15, 70, 15);
            updateMaxOrder();
        }
        else
        {
            displayTextIfNotSelectable = getOrderString (order) + " order";
        }

        addAndMakeVisible (&cbNormalization);
        cbNormalization.setJustificationType (juce::Justification::centred);
        cbNormalization.addSectionHeading ("Normalization");
        cbNormalization.addItem ("N3D", 1);
        cbNormalization.addItem ("SN3D", 2);
        cbNormalization.setBounds (35, 0, 70, 15);
    }

    ~AmbisonicIOWidget() override {}

    void setOrderIfUnselectable (int newOrder)
    {
        if (! selectable && orderIfNotSelectable != newOrder)
        {
            orderIfNotSelectable = newOrder;
            updateDisplayTextIfNotSelectable();
        }
    }

    void updateDisplayTextIfNotSelectable()
    {
        if (maxPossibleOrder < orderIfNotSelectable)
        {
            displayTextIfNotSelectable = getOrderString (orderIfNotSelectable) + " (bus too small)";
            setBusTooSmall (true);
        }
        else
        {
            displayTextIfNotSelectable = getOrderString (orderIfNotSelectable) + " order";
            setBusTooSmall (false);
        }
        repaint();
    }

    void updateMaxOrder()
    {
        const int previousIndex = cbOrder.getSelectedItemIndex();
        cbOrder.clear();
        cbOrder.addSectionHeading ("Ambisonic Order");
        cbOrder.addItem ("Auto", 1);
        for (int o = 0; o <= maxOrder; ++o)
            cbOrder.addItem (getOrderString (o), o + 2);

        cbOrder.setSelectedItemIndex (previousIndex);
    }

    void setMaxOrder (int newMaxOrder)
    {
        maxOrder = newMaxOrder;
        updateMaxOrder();
        const int savedMaxPossibleOrder = maxPossibleOrder;
        maxPossibleOrder = -1;
        setMaxSize (savedMaxPossibleOrder);
    }

    const int getComponentSize() override { return 110; }

    /** Sets the maximally available size of the processor for this Widget.
     */
    void setMaxSize (int newMaxPossibleOrder) override
    {
        if (maxPossibleOrder != juce::jmin (newMaxPossibleOrder, maxOrder))
        {
            maxPossibleOrder = juce::jmin (newMaxPossibleOrder, maxOrder);

            if (selectable)
            {
                if (maxPossibleOrder > -1)
                    cbOrder.changeItemText (1, "Auto (" + getOrderString (maxPossibleOrder) + ")");
                else
                    cbOrder.changeItemText (1, "(Auto)");

                int currId = cbOrder.getSelectedId();
                if (currId == 0)
                    currId = 1; //bad work around

                for (int i = 1; i <= maxPossibleOrder; ++i)
                    cbOrder.changeItemText (i + 2, getOrderString (i));

                for (int i = maxPossibleOrder + 1; i <= maxOrder; ++i)
                    cbOrder.changeItemText (i + 2, getOrderString (i) + " (bus too small)");

                DBG (cbOrder.getItemText (cbOrder.indexOfItemId ((currId))));
                cbOrder.setText (cbOrder.getItemText (cbOrder.indexOfItemId ((currId))));
                if (currId - 2 > maxPossibleOrder)
                    setBusTooSmall (true);
                else
                    setBusTooSmall (false);
            }
            else
            {
                updateDisplayTextIfNotSelectable();
            }
        }
    }

    juce::ComboBox* getNormCbPointer() { return &cbNormalization; }
    juce::ComboBox* getOrderCbPointer()
    {
        if (! selectable)
            // There's no Ambisonic Order ComboBox, when order is not selectable!
            jassertfalse;

        return &cbOrder;
    }

    void paint (juce::Graphics& g) override
    {
        AmbiLogoPath.applyTransform (
            AmbiLogoPath
                .getTransformToScaleToFit (0, 0, 30, 30, true, juce::Justification::centred));
        g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (AmbiLogoPath);

        if (! selectable)
        {
            g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
            g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 1)));
            g.setFont (15.0f);
            g.drawFittedText (displayTextIfNotSelectable,
                              35,
                              15,
                              55,
                              15,
                              juce::Justification::centred,
                              1);
        }
    };

private:
    juce::ComboBox cbNormalization, cbOrder;
    juce::Path AmbiLogoPath;
    int maxOrder = order;
    int orderIfNotSelectable = order;
    int maxPossibleOrder = -1;
    juce::String displayTextIfNotSelectable;
};

class DirectivityIOWidget : public IOWidget
{
public:
    DirectivityIOWidget() : IOWidget()
    {
        DirectivityPath.loadPathFromData (DirectivityPathData, sizeof (DirectivityPathData));
        setBufferedToImage (true);
        orderStrings[0] = juce::String ("0th");
        orderStrings[1] = juce::String ("1st");
        orderStrings[2] = juce::String ("2nd");
        orderStrings[3] = juce::String ("3rd");
        orderStrings[4] = juce::String ("4th");
        orderStrings[5] = juce::String ("5th");
        orderStrings[6] = juce::String ("6th");
        orderStrings[7] = juce::String ("7th");

        addAndMakeVisible (&cbOrder);
        cbOrder.setJustificationType (juce::Justification::centred);
        cbOrder.addSectionHeading ("Directivity Order");
        cbOrder.addItem ("Auto", 1);
        cbOrder.addItem ("0th", 2);
        cbOrder.addItem ("1st", 3);
        cbOrder.addItem ("2nd", 4);
        cbOrder.addItem ("3rd", 5);
        cbOrder.addItem ("4th", 6);
        cbOrder.addItem ("5th", 7);
        cbOrder.addItem ("6th", 8);
        cbOrder.addItem ("7th", 9);
        cbOrder.setBounds (35, 15, 70, 15);

        addAndMakeVisible (&cbNormalization);
        cbNormalization.setJustificationType (juce::Justification::centred);
        cbNormalization.addSectionHeading ("Normalization");
        cbNormalization.addItem ("N3D", 1);
        cbNormalization.addItem ("SN3D", 2);
        cbNormalization.setBounds (35, 0, 70, 15);
    }

    const int getComponentSize() override { return 110; }

    void setMaxSize (int maxPossibleOrder) override
    {
        if (maxPossibleOrder > -1)
            cbOrder.changeItemText (1, "Auto (" + orderStrings[maxPossibleOrder] + ")");
        else
            cbOrder.changeItemText (1, "(Auto)");
        int currId = cbOrder.getSelectedId();
        if (currId == 0)
            currId = 1; //bad work around
        int i;
        for (i = 1; i <= maxPossibleOrder; ++i)
        {
            cbOrder.changeItemText (i + 2, orderStrings[i]);
        }
        for (i = maxPossibleOrder + 1; i <= 7; ++i)
        {
            cbOrder.changeItemText (i + 2, orderStrings[i] + " (bus too small)");
        }
        cbOrder.setText (cbOrder.getItemText (cbOrder.indexOfItemId (currId)));
        if (currId - 2 > maxPossibleOrder)
            setBusTooSmall (true);
        else
            setBusTooSmall (false);
    }

    juce::ComboBox* getNormCbPointer() { return &cbNormalization; }
    juce::ComboBox* getOrderCbPointer() { return &cbOrder; }

    void paint (juce::Graphics& g) override
    {
        DirectivityPath.applyTransform (
            DirectivityPath
                .getTransformToScaleToFit (0, 0, 30, 30, true, juce::Justification::centred));
        g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
        g.fillPath (DirectivityPath);
    }

private:
    juce::String orderStrings[8];
    juce::ComboBox cbNormalization, cbOrder;
    juce::Path DirectivityPath;
};

// ======================================================== TITLEBAR =========================
template <class Tin, class Tout>
class TitleBar : public juce::Component
{
public:
    TitleBar() : juce::Component()
    {
        addAndMakeVisible (&inputWidget);
        addAndMakeVisible (&outputWidget);
    }

    Tin* getInputWidgetPtr() { return &inputWidget; }
    Tout* getOutputWidgetPtr() { return &outputWidget; }

    void setTitle (juce::String newBoldText, juce::String newRegularText)
    {
        boldText = newBoldText;
        regularText = newRegularText;
    }

    void setFont (juce::Typeface::Ptr newBoldFont, juce::Typeface::Ptr newRegularFont)
    {
        boldFont = newBoldFont;
        regularFont = newRegularFont;
    }

    void resized() override
    {
        const int leftWidth = inputWidget.getComponentSize();
        const int rightWidth = outputWidget.getComponentSize();

        inputWidget.setBounds (getLocalBounds().removeFromLeft (leftWidth).reduced (0, 15));
        outputWidget.setBounds (getLocalBounds().removeFromRight (rightWidth).reduced (0, 15));
    }

    void setMaxSize (std::pair<int, int> inOutSizes)
    {
        inputWidget.setMaxSize (inOutSizes.first);
        outputWidget.setMaxSize (inOutSizes.second);
    }

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        const float centreX = bounds.getX() + bounds.getWidth() * 0.5f;
        const float centreY = bounds.getY() + bounds.getHeight() * 0.5f;
        const float boldHeight = 25.f;
        const float regularHeight = 25.f;
        const int leftWidth = inputWidget.getComponentSize();
        const int rightWidth = outputWidget.getComponentSize();

        boldFont.setHeight (boldHeight);
        regularFont.setHeight (regularHeight);

        const float boldWidth = boldFont.getStringWidth (boldText);
        const float regularWidth = regularFont.getStringWidth (regularText);

        juce::Rectangle<float> textArea (0,
                                         0,
                                         boldWidth + regularWidth,
                                         juce::jmax (boldHeight, regularHeight));
        textArea.setCentre (centreX, centreY);

        if (textArea.getX() < leftWidth)
            textArea.setX (leftWidth);
        if (textArea.getRight() > bounds.getRight() - rightWidth)
            textArea.setRight (bounds.getRight() - rightWidth);

        g.setColour (juce::Colours::white);
        g.setFont (boldFont);
        g.drawFittedText (boldText,
                          textArea.removeFromLeft (boldWidth).toNearestInt(),
                          juce::Justification::bottom,
                          1);
        g.setFont (regularFont);
        g.drawFittedText (regularText, textArea.toNearestInt(), juce::Justification::bottom, 1);

        g.setColour ((juce::Colours::white).withMultipliedAlpha (0.5));
        g.drawLine (bounds.getX(),
                    bounds.getY() + bounds.getHeight() - 4,
                    bounds.getX() + bounds.getWidth(),
                    bounds.getY() + bounds.getHeight() - 4);
    }

private:
    Tin inputWidget;
    Tout outputWidget;
    juce::Font boldFont = juce::Font (25.f);
    juce::Font regularFont = juce::Font (25.f);
    juce::String boldText = "Bold";
    juce::String regularText = "Regular";
};

class IEMLogo : public juce::Component
{
public:
    IEMLogo() : juce::Component()
    {
        IEMPath.loadPathFromData (IEMpathData, sizeof (IEMpathData));
        url = juce::URL ("https://plugins.iem.at/");
    }

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        bounds.removeFromBottom (3);
        bounds.removeFromLeft (1);
        IEMPath.applyTransform (IEMPath.getTransformToScaleToFit (bounds.reduced (2, 2).toFloat(),
                                                                  true,
                                                                  juce::Justification::bottomLeft));

        if (isMouseOver())
        {
            g.setColour (juce::Colour::fromRGB (52, 88, 165));
            g.fillAll();
        }

        g.setColour (isMouseOver() ? juce::Colour::fromRGB (249, 226, 45)
                                   : juce::Colours::white.withMultipliedAlpha (0.5));
        g.fillPath (IEMPath);
    }

    void mouseEnter (const juce::MouseEvent& event) override
    {
        juce::ignoreUnused (event);
        setMouseCursor (juce::MouseCursor (juce::MouseCursor::PointingHandCursor));
        repaint();
    }

    void mouseExit (const juce::MouseEvent& event) override
    {
        juce::ignoreUnused (event);
        setMouseCursor (juce::MouseCursor (juce::MouseCursor::NormalCursor));
        repaint();
    }

    void mouseUp (const juce::MouseEvent& event) override
    {
        juce::ignoreUnused (event);
        if (url.isWellFormed())
            url.launchInDefaultBrowser();
    }

private:
    juce::Path IEMPath;
    juce::URL url;
};

class Footer : public juce::Component
{
public:
    Footer() : juce::Component() { addAndMakeVisible (&iemLogo); }

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 0)));
        g.setFont (14.0f);
        juce::String versionString = "v";

#if JUCE_DEBUG
        versionString = "DEBUG - v";
#endif
        versionString.append (JucePlugin_VersionString, 6);

        g.drawText (versionString,
                    0,
                    0,
                    bounds.getWidth() - 8,
                    bounds.getHeight() - 2,
                    juce::Justification::bottomRight);
    }

    void resized() override { iemLogo.setBounds (0, 0, 40, getHeight()); }

private:
    IEMLogo iemLogo;
};

#ifdef JUCE_OSC_H_INCLUDED
class OSCFooter : public juce::Component
{
public:
    OSCFooter (OSCParameterInterface& oscInterface) : oscStatus (oscInterface)
    {
        addAndMakeVisible (footer);
        addAndMakeVisible (oscStatus);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        footer.setBounds (bounds);

        bounds.removeFromBottom (2);
        bounds = bounds.removeFromBottom (16);
        bounds.removeFromLeft (50);
        oscStatus.setBounds (bounds);
    }

private:
    OSCStatus oscStatus;
    Footer footer;
};
#endif
