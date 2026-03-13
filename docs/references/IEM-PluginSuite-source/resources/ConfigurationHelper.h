/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich, Leo McCormack
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

#ifndef CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS
    #define CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS 0
#endif

#ifndef CONFIGURATIONHELPER_ENABLE_DECODER_METHODS
    #define CONFIGURATIONHELPER_ENABLE_DECODER_METHODS 0
#endif

#ifndef CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS
    #define CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS 0
#else
    #undef CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS
    #define CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS 1
#endif

#ifndef CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS
    #define CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS 0
#endif

#ifndef CONFIGURATIONHELPER_ENABLE_ALL_METHODS
    #define CONFIGURATIONHELPER_ENABLE_ALL_METHODS 0
#endif

#if CONFIGURATIONHELPER_ENABLE_DECODER_METHODS
    #undef CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS
    #define CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS 1
#endif

#if CONFIGURATIONHELPER_ENABLE_ALL_METHODS
    #undef CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS
    #define CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS 1

    #undef CONFIGURATIONHELPER_ENABLE_DECODER_METHODS
    #define CONFIGURATIONHELPER_ENABLE_DECODER_METHODS 1

    #undef CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS
    #define CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS 1

    #undef CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS
    #define CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS 1
#endif

#if CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS
    #include "ReferenceCountedMatrix.h"
#endif

#if CONFIGURATIONHELPER_ENABLE_DECODER_METHODS
    #include "ReferenceCountedDecoder.h"
#endif
//==============================================================================
/**
 This class contains some helper-functions for loading / storing Decoders, Matrices and LoudspeakerLayouts from / as JSON configuration files.
 This file should be included into a JUCE project as it has JUCE dependencies.
 */

class ConfigurationHelper
{
public:
    // =============== IMPORT ======================================================
    /**
     Loads a JSON-file (fileToParse) and writes the parsed content to a juce::var object (dest).
     */
    static juce::Result parseFile (const juce::File& fileToParse, juce::var& dest)
    {
        if (! fileToParse.exists())
            return juce::Result::fail ("File '" + fileToParse.getFullPathName()
                                       + "' does not exist!");

        juce::String jsonString = fileToParse.loadFileAsString();
        juce::Result result = juce::JSON::parse (jsonString, dest);
        if (! result.wasOk())
            return juce::Result::fail ("File '" + fileToParse.getFullPathName()
                                       + "' could not be parsed:\n" + result.getErrorMessage());

        return juce::Result::ok();
    }

#if CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS
    /**
     Loads a JSON-file (fileToParse) and tries to parse for a TransformationMatrix object. If successful, writes the matrix into the destination (dest).
     */
    static juce::Result parseFileForTransformationMatrix (const juce::File& fileToParse,
                                                          ReferenceCountedMatrix::Ptr* dest)
    {
        jassert (dest != nullptr);

        // parsing configuration file
        juce::var parsedJson;
        {
            juce::Result result = parseFile (fileToParse, parsedJson);
            if (! result.wasOk())
                return juce::Result::fail (result.getErrorMessage());
        }

        // looking for a 'TransformationMatrix' object
        juce::var tmVar = parsedJson.getProperty ("TransformationMatrix", parsedJson);
        juce::Result result = convertTransformationMatrixVarToMatrix (
            tmVar,
            dest,
            parsedJson.getProperty ("Name", juce::var ("")),
            parsedJson.getProperty ("Description", juce::var ("")));

        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        return juce::Result::ok();
    }

    /**
     Converts the juce::dsp::Matrix object within a TransformationMatrix juce::var object (tmVar) to a ReferenceCountedMatrix (matrix).
     */
    static juce::Result
        convertTransformationMatrixVarToMatrix (juce::var& tmVar,
                                                ReferenceCountedMatrix::Ptr* matrix,
                                                juce::var nameFallback = juce::var (""),
                                                juce::var descriptionFallback = juce::var (""))
    {
        jassert (matrix != nullptr);

        juce::String name = tmVar.getProperty (juce::Identifier ("Name"), nameFallback);
        juce::String description =
            tmVar.getProperty (juce::Identifier ("Description"), descriptionFallback);

        if (! tmVar.hasProperty ("Matrix"))
            return juce::Result::fail ("There is no 'Matrix' array.");

        int rows, cols;
        juce::var matrixData = tmVar.getProperty ("Matrix", juce::var());
        auto result = getMatrixDataSize (matrixData, rows, cols);

        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        ReferenceCountedMatrix::Ptr newMatrix =
            new ReferenceCountedMatrix (name, description, rows, cols);
        result = getMatrix (matrixData, rows, cols, newMatrix->getMatrix());

        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        *matrix = newMatrix;
        return juce::Result::ok();
    }

    /**
     Converts a 'Matrix' juce::var object to a juce::dsp::Matrix<float> object.
     */
    static juce::Result
        getMatrix (juce::var& matrixData, int rows, int cols, juce::dsp::Matrix<float>& dest)
    {
        for (int r = 0; r < rows; ++r)
        {
            auto rowVar = matrixData.getArray()->getUnchecked (r);
            if (rowVar.size() != cols)
                return juce::Result::fail ("Matrix row " + juce::String (r + 1)
                                           + " has wrong length (should be " + juce::String (cols)
                                           + ").");

            for (int c = 0; c < cols; ++c)
            {
                auto colVar = rowVar.getArray()->getUnchecked (c);
                if (colVar.isDouble() || colVar.isInt())
                {
                    dest (r, c) = colVar;
                }
                else
                    return juce::Result::fail ("Datatype of matrix element (" + juce::String (r + 1)
                                               + "," + juce::String (c + 1)
                                               + ") could not be parsed.");
            }
        }
        return juce::Result::ok();
    }

    /**
     Extracts the number of rows and columns out of a 'Matrix' juce::var object.
     */
    static juce::Result getMatrixDataSize (juce::var& matrixData, int& rows, int& cols)
    {
        rows = matrixData.size();
        cols = matrixData.getArray()->getUnchecked (0).size();

        return juce::Result::ok();
    }

#endif // #if CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS

#if CONFIGURATIONHELPER_ENABLE_DECODER_METHODS
    /**
     Converts a Decoder juce::var object (decoderVar) to a ReferenceCountedDecoder (decoder).
     */
    static juce::Result DecoderVar (juce::var& decoderVar,
                                    ReferenceCountedDecoder::Ptr* decoder,
                                    juce::var nameFallback = juce::var (""),
                                    juce::var descriptionFallback = juce::var (""))
    {
        jassert (decoder != nullptr);

        juce::String name = decoderVar.getProperty (juce::Identifier ("Name"), nameFallback);
        juce::String description =
            decoderVar.getProperty (juce::Identifier ("Description"), descriptionFallback);

        if (! decoderVar.hasProperty ("Matrix"))
            return juce::Result::fail ("There is no 'Matrix' array within the 'Decoder' object.");

        // get matrix size
        int rows, cols;
        auto matrixData = decoderVar.getProperty ("Matrix", juce::var());
        auto result = getMatrixDataSize (matrixData, rows, cols);
        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        //check if cols is a valid number of Ambisonic channels
        const int decoderOrder = sqrt (cols) - 1;
        if (cols != juce::square (decoderOrder + 1))
            return juce::Result::fail (
                "Decoder matrix's number of columns is no valid Ambisonic channel count: nCh = (order+1)^2.");

        // create decoder and get matrix from 'Decoder' object
        ReferenceCountedDecoder::Ptr newDecoder =
            new ReferenceCountedDecoder (name, description, rows, cols);
        result = getMatrix (matrixData, rows, cols, newDecoder->getMatrix());
        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        if (decoderVar.hasProperty ("Routing"))
        {
            auto routingData = decoderVar.getProperty ("Routing", juce::var());
            result = getRoutingArray (routingData, rows, newDecoder);
            if (! result.wasOk())
                return juce::Result::fail (result.getErrorMessage());
        }

        // ============ SETTINGS =====================
        ReferenceCountedDecoder::Settings settings;
        // normalization
        if (! decoderVar.hasProperty ("ExpectedInputNormalization"))
            juce::Result::fail ("Could not find 'ExpectedInputNormalization' attribute.");

        auto expectedNormalization (
            decoderVar.getProperty ("ExpectedInputNormalization", juce::var()));
        if (expectedNormalization.toString().equalsIgnoreCase ("sn3d"))
            settings.expectedNormalization = ReferenceCountedDecoder::Normalization::sn3d;
        else if (expectedNormalization.toString().equalsIgnoreCase ("n3d"))
            settings.expectedNormalization = ReferenceCountedDecoder::Normalization::n3d;
        else
            return juce::Result::fail (
                "Could not parse 'ExpectedInputNormalization' attribute. Expected 'sn3d' or 'n3d' but got '"
                + expectedNormalization.toString() + "'.");

        // weights
        if (decoderVar.hasProperty ("Weights"))
        {
            auto weights (decoderVar.getProperty ("Weights", juce::var()));
            if (weights.toString().equalsIgnoreCase ("maxrE"))
                settings.weights = ReferenceCountedDecoder::Weights::maxrE;
            else if (weights.toString().equalsIgnoreCase ("inPhase"))
                settings.weights = ReferenceCountedDecoder::Weights::inPhase;
            else if (weights.toString().equalsIgnoreCase ("none"))
                settings.weights = ReferenceCountedDecoder::Weights::none;
            else
                return juce::Result::fail (
                    "Could not parse 'Weights' attribute. Expected 'maxrE', 'inPhase' or 'none' but got '"
                    + weights.toString() + "'.");
        }

        // weights already applied
        if (decoderVar.hasProperty ("WeightsAlreadyApplied"))
        {
            auto weightsAlreadyApplied (
                decoderVar.getProperty ("WeightsAlreadyApplied", juce::var()));
            if (weightsAlreadyApplied.isBool())
                settings.weightsAlreadyApplied = weightsAlreadyApplied;
            else
                return juce::Result::fail (
                    "Could not parse 'WeightsAlreadyApplied' attribute. Expected bool but got '"
                    + weightsAlreadyApplied.toString() + "'.");
        }
        if (decoderVar.hasProperty ("SubwooferChannel"))
        {
            auto subwooferChannel (decoderVar.getProperty ("SubwooferChannel", juce::var()));
            if (subwooferChannel.isInt())
            {
                if (static_cast<int> (subwooferChannel) < 1
                    || static_cast<int> (subwooferChannel) > 64)
                    return juce::Result::fail (
                        "'SubwooferChannel' attribute is not a valid channel number (1 <= subwooferChannel <= 64).");

                settings.subwooferChannel = subwooferChannel;
            }
            else
                return juce::Result::fail (
                    "Could not parse 'SubwooferChannel' attribute. Expected channel number (int) but got '"
                    + subwooferChannel.toString() + "'.");
        }

        newDecoder->setSettings (settings);

        *decoder = newDecoder;
        return juce::Result::ok();
    }

    /**
     Extracts the routing array from a 'Routing' Var object and writes it to the ReferenceCountedMatrix object.
     */
    static juce::Result
        getRoutingArray (juce::var& routingData, const int rows, ReferenceCountedMatrix::Ptr dest)
    {
        if (routingData.size() != rows)
            return juce::Result::fail (
                "Length of 'Routing' attribute does not match number of matrix outputs (rows).");

        auto& routingArray = dest->getRoutingArrayReference();
        for (int r = 0; r < rows; ++r)
        {
            auto element = routingData.getArray()->getUnchecked (r);

            if (element.isInt())
                routingArray.set (r, (int) element - 1);
            else
                return juce::Result::fail ("Datatype of 'Routing' element at position "
                                           + juce::String (r + 1)
                                           + " could not be interpreted (expected integer).");
        }
        return juce::Result::ok();
    }

    /**
     Loads a JSON-file (fileToParse) and tries to parse for a 'Decoder' object. If successful, writes the decoder into the destination (decoder).
     */
    static juce::Result parseFileForDecoder (const juce::File& fileToParse,
                                             ReferenceCountedDecoder::Ptr* decoder)
    {
        jassert (decoder != nullptr);

        // parsing configuration file
        juce::var parsedJson;
        juce::Result result = parseFile (fileToParse, parsedJson);
        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        result = parseVarForDecoder (parsedJson, decoder);
        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        return juce::Result::ok();
    }

    /** Parses a 'Decoder' object from a JSON juce::var. If successful, writes the decoder into the destination (decoder).
     */
    static juce::Result parseVarForDecoder (const juce::var& jsonVar,
                                            ReferenceCountedDecoder::Ptr* decoder)
    {
        jassert (decoder != nullptr);

        // looking for a 'Decoder' object
        if (! jsonVar.hasProperty ("Decoder"))
            return juce::Result::fail ("No 'Decoder' object found in the configuration file.");

        auto decoderObject = jsonVar.getProperty ("Decoder", jsonVar);
        auto result = DecoderVar (decoderObject,
                                  decoder,
                                  jsonVar.getProperty ("Name", juce::var ("")),
                                  jsonVar.getProperty ("Description", juce::var ("")));

        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        return juce::Result::ok();
    }

#endif //#if CONFIGURATIONHELPER_ENABLE_DECODER_METHODS

#if CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS
    /**
     Loads a JSON-file (fileToParse) and tries to parse for a 'LoudspeakerLayout' object. If successful, writes the loudspeakers (named 'elements') into a ValeTree object (loudspeakers). Set 'undoManager' to nullptr in case you don't want to use a undoManager.
     */
    static juce::Result parseFileForLoudspeakerLayout (const juce::File& fileToParse,
                                                       juce::ValueTree& loudspeakers,
                                                       juce::UndoManager* undoManager)
    {
        return parseFileForGenericLayout (fileToParse, loudspeakers, undoManager);
    }

#endif // #if CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS

#if CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS
    /**
     Loads a JSON-file (fileToParse) and tries to parse for a 'LoudspeakerLayout' or 'GenericLayout' object. If successful, writes the generic object into a juce::ValueTree object (elements). Set 'undoManager' to nullptr in case you don't want to use a undoManager.
     */
    static juce::Result parseFileForGenericLayout (const juce::File& fileToParse,
                                                   juce::ValueTree& elements,
                                                   juce::UndoManager* undoManager)
    {
        // parse configuration file
        juce::var parsedJson;
        juce::Result result = parseFile (fileToParse, parsedJson);
        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        // looks for a 'GenericLayout' or 'LoudspeakerLayout' object
        juce::var genericLayout;
        if (parsedJson.hasProperty ("GenericLayout"))
            genericLayout = parsedJson.getProperty ("GenericLayout", juce::var());
        else if (parsedJson.hasProperty ("LoudspeakerLayout"))
            genericLayout = parsedJson.getProperty ("LoudspeakerLayout", juce::var());
        else
            return juce::Result::fail (
                "No 'GenericLayout' or 'LoudspeakerLayout' object found in the configuration file.");

        // looks for a 'GenericLayout' or 'LoudspeakerLayout' object
        juce::var elementArray;
        if (genericLayout.hasProperty ("Elements"))
            elementArray = genericLayout.getProperty ("Elements", juce::var());
        else if (genericLayout.hasProperty ("Loudspeakers"))
            elementArray = genericLayout.getProperty ("Loudspeakers", juce::var());
        else
            return juce::Result::fail (
                "No 'Elements' or 'Loudspeakers' attribute found within the 'GenericLayout' or 'LoudspeakerLayout' object.");

        result = addElementsToValueTree (elementArray, elements, undoManager);

        if (! result.wasOk())
            return juce::Result::fail (result.getErrorMessage());

        return juce::Result::ok();
    }

    /**
     Appends all elements within the GenericLayout to the elements juce::ValueTree.
     */
    static juce::Result addElementsToValueTree (juce::var& elementArray,
                                                juce::ValueTree& elements,
                                                juce::UndoManager* undoManager)
    {
        if (! elementArray.isArray())
            return juce::Result::fail ("'elementArray' is not an array.");

        const int nElements = elementArray.size();

        for (int i = 0; i < nElements; ++i)
        {
            juce::var& element = elementArray[i];
            float azimuth, elevation, radius, gain;
            int channel;
            bool isImaginary;

            if (! element.hasProperty ("Azimuth"))
                return juce::Result::fail ("No 'Azimuth' attribute for element #"
                                           + juce::String (i + 1) + ".");
            auto azi = element.getProperty ("Azimuth", juce::var());
            if (azi.isDouble() || azi.isInt())
                azimuth = azi;
            else
                return juce::Result::fail ("Wrong datatype for attribute 'Azimuth' for element #"
                                           + juce::String (i + 1) + ".");

            if (! element.hasProperty ("Elevation"))
                return juce::Result::fail ("No 'Elevation' attribute for element #"
                                           + juce::String (i + 1) + ".");
            auto ele = element.getProperty ("Elevation", juce::var());
            if (ele.isDouble() || ele.isInt())
                elevation = ele;
            else
                return juce::Result::fail ("Wrong datatype for attribute 'Elevation' for element #"
                                           + juce::String (i + 1) + ".");

            if (! element.hasProperty ("Radius"))
                return juce::Result::fail ("No 'Radius' attribute for element #"
                                           + juce::String (i + 1) + ".");
            auto rad = element.getProperty ("Radius", juce::var());
            if (rad.isDouble() || rad.isInt())
                radius = rad;
            else
                return juce::Result::fail ("Wrong datatype for attribute 'Radius' for element #"
                                           + juce::String (i + 1) + ".");

            if (! element.hasProperty ("Gain"))
                return juce::Result::fail ("No 'Gain' attribute for element #"
                                           + juce::String (i + 1) + ".");
            auto g = element.getProperty ("Gain", juce::var());
            if (g.isDouble() || g.isInt())
                gain = g;
            else
                return juce::Result::fail ("Wrong datatype for attribute 'Gain' for element #"
                                           + juce::String (i + 1) + ".");

            if (! element.hasProperty ("Channel"))
                return juce::Result::fail ("No 'Channel' attribute for element #"
                                           + juce::String (i + 1) + ".");
            auto ch = element.getProperty ("Channel", juce::var());
            if (ch.isInt())
                channel = ch;
            else
                return juce::Result::fail ("Wrong datatype for attribute 'Channel' for element #"
                                           + juce::String (i + 1) + ".");

            if (! element.hasProperty ("IsImaginary"))
                return juce::Result::fail ("No 'IsImaginary' attribute for element #"
                                           + juce::String (i + 1) + ".");
            auto im = element.getProperty ("IsImaginary", juce::var());
            if (im.isBool())
                isImaginary = im;
            else
                return juce::Result::fail (
                    "Wrong datatype for attribute 'IsImaginary' for element #"
                    + juce::String (i + 1) + ".");

            elements.appendChild (
                createElement (azimuth, elevation, radius, channel, isImaginary, gain),
                undoManager);
        }

        return juce::Result::ok();
    }

    /**
     Creates a single element juce::ValueTree, which can be appended to another juce::ValueTree holding several elements.
     */
    static juce::ValueTree createElement (float azimuth,
                                          float elevation,
                                          float radius,
                                          int channel,
                                          bool isImaginary,
                                          float gain)
    {
        juce::ValueTree newElement ("Element");

        newElement.setProperty ("Azimuth", azimuth, nullptr);
        newElement.setProperty ("Elevation", elevation, nullptr);
        newElement.setProperty ("Radius", radius, nullptr);
        newElement.setProperty ("Channel", channel, nullptr);
        newElement.setProperty ("Imaginary", isImaginary, nullptr);
        newElement.setProperty ("Gain", gain, nullptr);

        return newElement;
    }

#endif // #if CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS

#if CONFIGURATIONHELPER_ENABLE_DECODER_METHODS
    // =============== EXPORT ======================================================
    /**
     Converts a ReferenceCountedDecoder object to a juce::var object. Useful for writing the Decoder to a configuration file.
     */
    static juce::var convertDecoderToVar (ReferenceCountedDecoder::Ptr& decoder)
    {
        if (decoder == nullptr)
            return juce::var();

        auto* obj = new juce::DynamicObject();
        obj->setProperty ("Name", decoder->getName());
        obj->setProperty ("Description", decoder->getDescription());

        ReferenceCountedDecoder::Settings settings = decoder->getSettings();

        obj->setProperty (
            "ExpectedInputNormalization",
            settings.expectedNormalization == ReferenceCountedDecoder::Normalization::n3d ? "n3d"
                                                                                          : "sn3d");

        obj->setProperty ("Weights",
                          settings.weights == ReferenceCountedDecoder::Weights::maxrE ? "maxrE"
                          : settings.weights == ReferenceCountedDecoder::Weights::inPhase
                              ? "inPhase"
                              : "none");
        obj->setProperty ("WeightsAlreadyApplied", juce::var (settings.weightsAlreadyApplied));

        const int subwooferChannel = settings.subwooferChannel;
        if (subwooferChannel > 0)
            obj->setProperty ("SubwooferChannel", subwooferChannel);

        // decoder matrix
        juce::var decoderMatrix = convertMatrixToVar (decoder->getMatrix());
        obj->setProperty ("Matrix", decoderMatrix);

        // routing array
        juce::var routing;
        juce::Array<int>& routingArray = decoder->getRoutingArrayReference();
        for (int i = 0; i < routingArray.size(); ++i)
            routing.append (routingArray[i] + 1); // one count

        obj->setProperty ("Routing", routing);

        return juce::var (obj);
    }

#endif // #if CONFIGURATIONHELPER_ENABLE_DECODER_METHODS

#if CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS
    /**
     Converts a juce::dsp::Matrix<float> object to a juce::var object.
     */
    static juce::var convertMatrixToVar (juce::dsp::Matrix<float>& mat)
    {
        juce::var matrixVar;
        for (int m = 0; m < mat.getSize()[0]; ++m)
        {
            juce::var row;
            for (int n = 0; n < mat.getSize()[1]; ++n)
                row.append (mat (m, n));

            matrixVar.append (row);
        }
        return matrixVar;
    }

    /**
     Converts a ReferenceCountedMatrix object to a juce::var object. Useful for writing the juce::dsp::Matrix to a configuration file.
     */
    static juce::var convertTransformationMatrixToVar (ReferenceCountedMatrix::Ptr& matrix)
    {
        if (matrix == nullptr)
            return juce::var();

        auto* obj = new juce::DynamicObject();
        obj->setProperty ("Name", matrix->getName());
        obj->setProperty ("Description", matrix->getDescription());

        auto transformationMatrix = convertMatrixToVar (matrix->getMatrix());

        obj->setProperty ("Matrix", transformationMatrix);
        return juce::var (obj);
    }

#endif // #if CONFIGURATIONHELPER_ENABLE_MATRIX_METHODS

#if CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS
    /**
     Converts a loudspeakers juce::ValueTree object to a juce::var object. Useful for writing the loudspeakers to a configuration file ('LoudspeakerLayout'). Make sure the juce::ValueTree contains valid loudspeakers.
     */
    static juce::var convertLoudspeakersToVar (juce::ValueTree& loudspeakers,
                                               juce::String name = "",
                                               juce::String description = "")
    {
        auto* obj = new juce::DynamicObject(); // loudspeaker layout object
        if (! name.isEmpty())
            obj->setProperty ("Name", name);
        if (! description.isEmpty())
            obj->setProperty ("Description", description);

        juce::var loudspeakerArray;

        for (juce::ValueTree::Iterator it = loudspeakers.begin(); it != loudspeakers.end(); ++it)
        {
            auto* loudspeaker = new juce::
                DynamicObject(); // loudspeaker which gets added to the loudspeakerArray juce::var

            loudspeaker->setProperty ("Azimuth", (*it).getProperty ("Azimuth"));
            loudspeaker->setProperty ("Elevation", (*it).getProperty ("Elevation"));
            loudspeaker->setProperty ("Radius", (*it).getProperty ("Radius"));
            loudspeaker->setProperty ("IsImaginary", (*it).getProperty ("Imaginary"));
            loudspeaker->setProperty ("Channel", (*it).getProperty ("Channel"));
            loudspeaker->setProperty ("Gain", (*it).getProperty ("Gain"));

            loudspeakerArray.append (juce::var (loudspeaker));
        }

        obj->setProperty ("Loudspeakers", loudspeakerArray);
        return juce::var (obj);
    }

#endif //#if CONFIGURATIONHELPER_ENABLE_LOUDSPEAKERLAYOUT_METHODS

#if CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS
    /**
     Converts a elements juce::ValueTree object to a juce::var object. Useful for writing the sources to a configuration file ('GenericLayout'). Make sure the juce::ValueTree contains valid elements.
     */
    static juce::var convertElementsToVar (juce::ValueTree& elements,
                                           juce::String name = "",
                                           juce::String description = "")
    {
        auto* obj = new juce::DynamicObject();
        if (! name.isEmpty())
            obj->setProperty ("Name", name);
        if (! description.isEmpty())
            obj->setProperty ("Description", description);

        juce::var elementArray;

        for (juce::ValueTree::Iterator it = elements.begin(); it != elements.end(); ++it)
        {
            auto* element = new juce::DynamicObject();

            element->setProperty ("Azimuth", (*it).getProperty ("Azimuth"));
            element->setProperty ("Elevation", (*it).getProperty ("Elevation"));
            element->setProperty ("Radius", (*it).getProperty ("Radius"));
            element->setProperty ("IsImaginary", (*it).getProperty ("Imaginary"));
            element->setProperty ("Channel", (*it).getProperty ("Channel"));
            element->setProperty ("Gain", (*it).getProperty ("Gain"));

            elementArray.append (juce::var (element));
        }

        obj->setProperty ("Elements", elementArray);
        return juce::var (obj);
    }

#endif //#if CONFIGURATIONHELPER_ENABLE_GENERICLAYOUT_METHODS

    /**
     Writes a configuration juce::var to a JSON file.
     Example use-case:
        DynamicObject* configuration = new DynamicObject();
        configuration->setProperty("Name", juce::var("Configuration Name"));
        configuration->setProperty("Description", juce::var("Description"));
        configuration->setProperty ("Decoder", ConfigurationHelper::convertDecoderToVar (referenceCountedDecoder));
        configuration->setProperty ("LoudspeakerLayout", ConfigurationHelper::convertLoudspeakersToVar (loudspeakersValueTree));
        ConfigurationHelper::writeConfigurationToFile (fileName, juce::var (configuration));
     */
    static juce::Result writeConfigurationToFile (juce::File& fileToWrite, juce::var configuration)
    {
        juce::String jsonString = juce::JSON::toString (configuration);
        if (fileToWrite.replaceWithText (jsonString))
            return juce::Result::ok();
        else
            return juce::Result::fail ("Writing configuration failed.");
    }
};
