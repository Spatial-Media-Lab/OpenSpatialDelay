#!/usr/bin/env node
/**
 * OpenSpatialDelay — Third-Party Legal Notices Generator
 *
 * Generates a standalone .docx document containing all required
 * third-party license notices and attributions for distributed
 * components (software libraries, HRTF datasets, fonts).
 *
 * Usage: node generate_legal_notices.js
 * Output: OpenSpatialDelay_Legal_Notices.docx
 */

const fs = require("fs");
const path = require("path");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  ImageRun, Header, Footer, AlignmentType, LevelFormat,
  ExternalHyperlink, HeadingLevel, BorderStyle, WidthType,
  ShadingType, PageNumber, PageBreak, TabStopType, TabStopPosition,
} = require("docx");

// ============================================================================
// CONSTANTS (matching generate_manual.js design system)
// ============================================================================

const DOCS_DIR = __dirname;
const ASSETS_DIR = path.join(DOCS_DIR, "assets");
const OUTPUT_PATH = path.join(DOCS_DIR, "OpenSpatialDelay_Legal_Notices.docx");

const C = {
  navy:       "001633",
  bodyText:   "1A1A2E",
  subheading: "334155",
  calloutBg:  "0A0D12",
  calloutTx:  "E1E5EA",
  cyan:       "80D8FF",
  violet:     "7457D1",
  tableHdr:   "001633",
  tableHdrTx: "FFFFFF",
  tableAlt:   "F8FAFC",
  tableBorder:"E2E8F0",
  dimText:    "64748B",
  white:      "FFFFFF",
  link:       "2563EB",
};

const A4_W = 11906;
const A4_H = 16838;
const MARGIN = 1440;
const CONTENT_W = A4_W - 2 * MARGIN;

const FONT_BODY = "DM Sans";
const FONT_MONO = "JetBrains Mono";

// ============================================================================
// ASSET LOADING
// ============================================================================

function loadImage(filename) {
  const p = path.join(ASSETS_DIR, filename);
  if (fs.existsSync(p)) return fs.readFileSync(p);
  return null;
}

const logoImg = loadImage("sml-logo.png");

// ============================================================================
// HELPERS
// ============================================================================

const thinBorder = { style: BorderStyle.SINGLE, size: 1, color: C.tableBorder };
const allThinBorders = { top: thinBorder, bottom: thinBorder, left: thinBorder, right: thinBorder };
const thickAccentBorder = (color) => ({ style: BorderStyle.SINGLE, size: 8, color });

function bodyText(text, opts = {}) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color: C.bodyText, ...opts });
}

function boldText(text, opts = {}) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color: C.bodyText, bold: true, ...opts });
}

function monoText(text, opts = {}) {
  return new TextRun({ text, font: FONT_MONO, size: 18, color: C.subheading, ...opts });
}

function linkText(text) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color: C.link, underline: {} });
}

function dimText(text) {
  return new TextRun({ text, font: FONT_BODY, size: 20, color: C.dimText, italics: true });
}

function bodyPara(children, opts = {}) {
  const runs = typeof children === "string" ? [bodyText(children)] : children;
  return new Paragraph({ spacing: { after: 160, line: 300 }, ...opts, children: runs });
}

function heading1(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_1,
    spacing: { before: 360, after: 240 },
    children: [new TextRun({ text, bold: true, font: FONT_BODY, size: 36, color: C.navy })],
  });
}

function heading2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2,
    spacing: { before: 280, after: 180 },
    children: [new TextRun({ text, bold: true, font: FONT_BODY, size: 30, color: C.navy })],
  });
}

function heading3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3,
    spacing: { before: 200, after: 120 },
    children: [new TextRun({ text, bold: true, font: FONT_BODY, size: 26, color: C.subheading })],
  });
}

function spacer(pts = 12) {
  return new Paragraph({ spacing: { after: pts * 20 } });
}

function externalLink(text, url) {
  return new ExternalHyperlink({ link: url, children: [linkText(text)] });
}

function dataTable(headers, rows, opts = {}) {
  const numCols = headers.length;
  const colWidths = opts.colWidths || headers.map(() => Math.floor(CONTENT_W / numCols));
  const sum = colWidths.reduce((a, b) => a + b, 0);
  if (sum !== CONTENT_W) colWidths[colWidths.length - 1] += (CONTENT_W - sum);

  const headerRow = new TableRow({
    tableHeader: true,
    children: headers.map((h, i) =>
      new TableCell({
        borders: allThinBorders,
        shading: { fill: C.tableHdr, type: ShadingType.CLEAR },
        margins: { top: 60, bottom: 60, left: 100, right: 100 },
        width: { size: colWidths[i], type: WidthType.DXA },
        children: [new Paragraph({
          children: [new TextRun({ text: h, font: FONT_BODY, size: 18, color: C.tableHdrTx, bold: true })],
        })],
      })
    ),
  });

  const dataRows = rows.map((row, ri) =>
    new TableRow({
      children: row.map((cell, ci) => {
        const cellChildren = typeof cell === "string"
          ? [new Paragraph({ children: [new TextRun({ text: cell, font: FONT_BODY, size: 18, color: C.bodyText })] })]
          : Array.isArray(cell) ? cell : [cell];
        return new TableCell({
          borders: allThinBorders,
          shading: ri % 2 === 1 ? { fill: C.tableAlt, type: ShadingType.CLEAR } : undefined,
          margins: { top: 50, bottom: 50, left: 100, right: 100 },
          width: { size: colWidths[ci], type: WidthType.DXA },
          children: cellChildren,
        });
      }),
    })
  );

  return new Table({
    width: { size: CONTENT_W, type: WidthType.DXA },
    columnWidths: colWidths,
    rows: [headerRow, ...dataRows],
  });
}

function licenseBlock(title, text) {
  return new Table({
    width: { size: CONTENT_W, type: WidthType.DXA },
    columnWidths: [CONTENT_W],
    rows: [
      new TableRow({
        children: [
          new TableCell({
            borders: {
              top: { style: BorderStyle.SINGLE, size: 1, color: "2A2E35" },
              bottom: { style: BorderStyle.SINGLE, size: 1, color: "2A2E35" },
              left: thickAccentBorder(C.cyan),
              right: { style: BorderStyle.SINGLE, size: 1, color: "2A2E35" },
            },
            shading: { fill: C.calloutBg, type: ShadingType.CLEAR },
            margins: { top: 120, bottom: 120, left: 200, right: 200 },
            width: { size: CONTENT_W, type: WidthType.DXA },
            children: [
              new Paragraph({
                spacing: { after: 100 },
                children: [new TextRun({ text: title, font: FONT_BODY, size: 22, color: C.cyan, bold: true })],
              }),
              ...text.split("\n\n").map(para =>
                new Paragraph({
                  spacing: { after: 80, line: 260 },
                  children: [new TextRun({ text: para, font: FONT_MONO, size: 16, color: C.calloutTx })],
                })
              ),
            ],
          }),
        ],
      }),
    ],
  });
}

// ============================================================================
// DOCUMENT CONTENT
// ============================================================================

function buildCover() {
  const items = [];

  items.push(spacer(40));

  if (logoImg) {
    items.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 200 },
      children: [new ImageRun({
        type: "png",
        data: logoImg,
        transformation: { width: 180, height: 91 },
        altText: { title: "Spatial Media Lab Logo", description: "Logo for Spatial Media Lab", name: "sml-logo" },
      })],
    }));
  }

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 80 },
    children: [new TextRun({ text: "OpenSpatialDelay", font: FONT_BODY, size: 72, color: C.navy, bold: true })],
  }));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 40 },
    children: [new TextRun({ text: "Third-Party Notices and Attributions", font: FONT_BODY, size: 32, color: C.subheading })],
  }));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 300 },
    children: [new TextRun({ text: "v1.0  |  April 2026", font: FONT_BODY, size: 24, color: C.dimText })],
  }));

  items.push(spacer(20));

  items.push(bodyPara([
    bodyText("This document lists all third-party software libraries, datasets, and fonts incorporated into OpenSpatialDelay, along with the license notices and attributions required by their respective licenses."),
  ], { alignment: AlignmentType.CENTER }));

  items.push(spacer(20));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    children: [
      externalLink("spatialmedialab.org", "https://spatialmedialab.org"),
      new TextRun({ text: "  |  ", font: FONT_BODY, size: 20, color: C.dimText }),
      externalLink("github.com/Spatial-Media-Lab/OpenSpatialDelay", "https://github.com/Spatial-Media-Lab/OpenSpatialDelay"),
    ],
  }));

  return items;
}

function buildSoftwareLibraries() {
  const items = [];
  items.push(heading1("Software Libraries"));

  items.push(bodyPara("OpenSpatialDelay is built with the following third-party software libraries:"));

  items.push(dataTable(
    ["Component", "Version", "License", "Copyright Holder"],
    [
      ["JUCE (Audio Plugin Framework)", "8.x", "GPL-3.0 / Commercial", "Raw Material Software Limited"],
      ["libmysofa (SOFA File Reader)", "1.3.2", "BSD-3-Clause", "Christian Hoene, Symonics GmbH"],
      ["zlib (Compression)", "1.x", "zlib License", "Jean-loup Gailly, Mark Adler"],
    ],
    { colWidths: [2800, 800, 2200, 3226] }
  ));
  items.push(spacer(12));

  // JUCE
  items.push(heading2("JUCE 8"));
  items.push(bodyPara([
    bodyText("JUCE is a cross-platform C++ framework for audio plugin development. It is dual-licensed under the GNU General Public License v3.0 and a commercial license from Raw Material Software Limited."),
  ]));
  items.push(bodyPara([
    bodyText("OpenSpatialDelay uses JUCE under the GPL-3.0 license. The full GPL-3.0 text is included in the LICENSE file distributed with the OpenSpatialDelay source code."),
  ]));
  items.push(bodyPara([
    bodyText("Website: "),
    externalLink("juce.com", "https://juce.com/"),
  ]));

  // libmysofa
  items.push(heading2("libmysofa v1.3.2"));
  items.push(bodyPara([
    bodyText("libmysofa is a C library for reading SOFA (Spatially Oriented Format for Acoustics) files containing HRTF measurements."),
  ]));
  items.push(bodyPara([
    bodyText("Website: "),
    externalLink("github.com/hoene/libmysofa", "https://github.com/hoene/libmysofa"),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("BSD 3-Clause License", `Copyright (c) 2016-2024 Christian Hoene, Symonics GmbH

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Symonics GmbH nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.`));
  items.push(spacer(12));

  // zlib
  items.push(heading2("zlib"));
  items.push(bodyPara([
    bodyText("zlib is a general-purpose compression library used by libmysofa for reading compressed SOFA files."),
  ]));
  items.push(bodyPara([
    bodyText("Website: "),
    externalLink("zlib.net", "https://www.zlib.net/"),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("zlib License", `Copyright (c) 1995-2024 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.`));
  items.push(spacer(12));

  return items;
}

function buildHRTFDatasets() {
  const items = [];
  items.push(heading1("HRTF Datasets"));

  items.push(bodyPara("OpenSpatialDelay embeds five SOFA (Spatially Oriented Format for Acoustics) datasets for Head-Related Transfer Function (HRTF) binaural rendering. Each dataset was measured from a different head/microphone and provides a distinct spatial character."));

  items.push(dataTable(
    ["Profile", "Dataset", "License", "Source Institution"],
    [
      ["Immersive", "SADIE II D2 KU100", "Apache License 2.0", "University of York"],
      ["Natural", "CIPIC Subject 003", "Public Domain", "UC Davis CIPIC Lab"],
      ["Precise", "HUTUBS PP2", "CC BY 4.0", "TU Berlin"],
      ["Spatial", "Bernschuetz KU100 2\u00b0", "CC BY 3.0", "TH K\u00f6ln"],
      ["Studio Reference", "MIT KEMAR Large Pinna", "MIT License", "MIT Media Lab"],
    ],
    { colWidths: [1600, 2400, 1800, 3226] }
  ));
  items.push(spacer(8));

  items.push(bodyPara([
    dimText("Note: The Simple (Low CPU) profile uses an internal Woodworth head model and does not incorporate any third-party data."),
  ]));
  items.push(spacer(8));

  // MIT KEMAR
  items.push(heading2("MIT KEMAR Large Pinna"));
  items.push(bodyPara([
    bodyText("HRTF measurements of a KEMAR mannequin with large pinnae, captured by the MIT Media Lab. Used as the Studio Reference profile."),
  ]));
  items.push(bodyPara([
    bodyText("Reference: "),
    externalLink("sound.media.mit.edu/resources/KEMAR.html", "https://sound.media.mit.edu/resources/KEMAR.html"),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("MIT License", `Copyright (c) Massachusetts Institute of Technology

Permission is hereby granted, free of charge, to any person obtaining a copy of this data and associated documentation files (the "Data"), to deal in the Data without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Data, and to permit persons to whom the Data is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Data.

THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.`));
  items.push(spacer(12));

  // SADIE II
  items.push(heading2("SADIE II D2 KU100"));
  items.push(bodyPara([
    bodyText("HRTF measurements of a Neumann KU100 dummy head captured as part of the SADIE II project at the University of York. Used as the Immersive profile."),
  ]));
  items.push(bodyPara([
    bodyText("Reference: Gavin Kearney, Tony Doyle, "),
    externalLink("york.ac.uk/sadie-project/database2", "https://www.york.ac.uk/sadie-project/database2.html"),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("Apache License 2.0 (Summary)", `Copyright (c) University of York

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at:

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

The full Apache License 2.0 text is available at the URL above.`));
  items.push(spacer(12));

  // CIPIC
  items.push(heading2("CIPIC Subject 003"));
  items.push(bodyPara([
    bodyText("HRTF measurements of a human subject from the CIPIC (Center for Image Processing and Integrated Computing) Interface Laboratory at UC Davis. Used as the Natural profile."),
  ]));
  items.push(bodyPara([
    bodyText("This dataset is in the "),
    boldText("public domain"),
    bodyText(" and may be used without restriction."),
  ]));
  items.push(bodyPara([
    bodyText("Reference: "),
    externalLink("ece.ucdavis.edu/cipic", "https://www.ece.ucdavis.edu/cipic/"),
  ]));
  items.push(spacer(12));

  // HUTUBS
  items.push(heading2("HUTUBS PP2"));
  items.push(bodyPara([
    bodyText("HRTF measurements from the HUTUBS (Head-related Utility for Binaural Synthesis) database by the Audio Communication Group at TU Berlin. Used as the Precise profile."),
  ]));
  items.push(bodyPara([
    bodyText("Reference: Fabian Brinkmann, Alexander Lindau, Stefan Weinzierl, et al. "),
    externalLink("depositonce.tu-berlin.de", "https://depositonce.tu-berlin.de/items/21a596ec-b7af-4e48-a8ed-4c4d68ce57a5"),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("Creative Commons Attribution 4.0 International (CC BY 4.0)", `This work is licensed under the Creative Commons Attribution 4.0 International License.

You are free to:
  - Share: copy and redistribute the material in any medium or format
  - Adapt: remix, transform, and build upon the material for any purpose, even commercially

Under the following terms:
  - Attribution: You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.

Full license text: https://creativecommons.org/licenses/by/4.0/legalcode`));
  items.push(spacer(12));

  // Bernschuetz
  items.push(heading2("Bernsch\u00fctz KU100 2\u00b0"));
  items.push(bodyPara([
    bodyText("High-resolution (2\u00b0 grid) HRTF measurements of a Neumann KU100 dummy head by Benjamin Bernsch\u00fctz at TH K\u00f6ln (Cologne University of Applied Sciences). Used as the Spatial profile."),
  ]));
  items.push(bodyPara([
    bodyText("Reference: "),
    externalLink("audiogroup.web.th-koeln.de", "https://audiogroup.web.th-koeln.de/"),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("Creative Commons Attribution 3.0 Unported (CC BY 3.0)", `This work is licensed under the Creative Commons Attribution 3.0 Unported License.

You are free to:
  - Share: copy and redistribute the material in any medium or format
  - Adapt: remix, transform, and build upon the material for any purpose, even commercially

Under the following terms:
  - Attribution: You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.

Full license text: https://creativecommons.org/licenses/by/3.0/legalcode`));
  items.push(spacer(12));

  return items;
}

function buildFonts() {
  const items = [];
  items.push(heading1("Fonts"));

  items.push(bodyPara("OpenSpatialDelay embeds the following open-source fonts for its user interface:"));

  items.push(dataTable(
    ["Font Family", "Weights Included", "License", "Copyright Holder"],
    [
      ["DM Sans", "Regular, Medium, SemiBold, Bold", "SIL OFL 1.1", "Colophon Foundry, Google"],
      ["JetBrains Mono", "Regular, Medium, Bold", "SIL OFL 1.1", "JetBrains s.r.o."],
      ["Roboto", "Medium", "Apache 2.0", "Google LLC"],
    ],
    { colWidths: [1800, 2800, 1400, 3026] }
  ));
  items.push(spacer(12));

  // OFL
  items.push(heading2("SIL Open Font License 1.1"));
  items.push(bodyPara([
    bodyText("DM Sans and JetBrains Mono are licensed under the SIL Open Font License, Version 1.1."),
  ]));
  items.push(spacer(8));

  items.push(licenseBlock("SIL Open Font License 1.1", `Copyright (c) Colophon Foundry (DM Sans), JetBrains s.r.o. (JetBrains Mono)

PREAMBLE
The goals of the Open Font License (OFL) are to stimulate worldwide development of collaborative font projects, to support the font creation efforts of academic and linguistic communities, and to provide a free and open framework in which fonts may be shared and improved in partnership with others.

The fonts are licensed, not sold, under this license.

PERMISSION & CONDITIONS
Permission is hereby granted, free of charge, to any person obtaining a copy of the Font Software, to use, study, copy, merge, embed, modify, redistribute, and sell modified and unmodified copies of the Font Software, subject to the following conditions:

1) Neither the Font Software nor any of its individual components, in Original or Modified Versions, may be sold by itself.

2) Original or Modified Versions of the Font Software may be bundled, redistributed and/or sold with any software, provided that each copy contains the above copyright notice and this license. These can be included either as stand-alone text files, human-readable headers or in the appropriate machine-readable metadata fields within text or binary files as long as those fields can be easily viewed by the user.

3) No Modified Version of the Font Software may use the Reserved Font Name(s) unless explicit written permission is granted by the corresponding Copyright Holder.

4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font Software shall not be used to promote, endorse or advertise any Modified Version, except to acknowledge the contribution(s) of the Copyright Holder(s) and the Author(s) or with their explicit written permission.

5) The Font Software, modified or unmodified, in part or in whole, must be distributed entirely under this license, and must not be distributed under any other license.

TERMINATION
This license becomes null and void if any of the above conditions are not met.

DISCLAIMER
THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY ARISING FROM THE USE OF THE FONT SOFTWARE.

Full license: https://openfontlicense.org/`));
  items.push(spacer(12));

  // Apache for Roboto
  items.push(heading2("Roboto (Apache License 2.0)"));
  items.push(bodyPara([
    bodyText("Roboto is licensed under the Apache License, Version 2.0 by Google LLC. You may obtain a copy of the License at "),
    externalLink("apache.org/licenses/LICENSE-2.0", "https://www.apache.org/licenses/LICENSE-2.0"),
    bodyText("."),
  ]));
  items.push(spacer(12));

  return items;
}

function buildProjectLicense() {
  const items = [];
  items.push(heading1("Project License"));

  items.push(bodyPara([
    bodyText("OpenSpatialDelay is free software, licensed under the "),
    boldText("GNU General Public License v3.0 (GPL-3.0)"),
    bodyText(". Any derivative work must also be released under the GPL-3.0. The full license text is included in the LICENSE file distributed with the source code."),
  ]));

  items.push(bodyPara([
    bodyText("Copyright (C) 2026 Spatial Media Lab ("),
    externalLink("spatialmedialab.org", "https://spatialmedialab.org"),
    bodyText(")"),
  ]));
  items.push(bodyPara([
    bodyText("Author: Andrew Rahman"),
  ]));

  items.push(spacer(20));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    children: [dimText("End of Third-Party Notices")],
  }));

  return items;
}

// ============================================================================
// DOCUMENT ASSEMBLY
// ============================================================================

async function buildDocument() {
  const coverContent = buildCover();
  const mainContent = [
    ...buildSoftwareLibraries(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildHRTFDatasets(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildFonts(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildProjectLicense(),
  ];

  const styles = {
    default: {
      document: { run: { font: FONT_BODY, size: 22 } },
    },
    paragraphStyles: [
      {
        id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 36, bold: true, font: FONT_BODY, color: C.navy },
        paragraph: { spacing: { before: 360, after: 240 }, outlineLevel: 0 },
      },
      {
        id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 30, bold: true, font: FONT_BODY, color: C.navy },
        paragraph: { spacing: { before: 280, after: 180 }, outlineLevel: 1 },
      },
      {
        id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 26, bold: true, font: FONT_BODY, color: C.subheading },
        paragraph: { spacing: { before: 200, after: 120 }, outlineLevel: 2 },
      },
    ],
  };

  const doc = new Document({
    styles,
    sections: [
      // Cover page
      {
        properties: {
          page: {
            size: { width: A4_W, height: A4_H },
            margin: { top: MARGIN, right: MARGIN, bottom: MARGIN, left: MARGIN },
          },
        },
        children: coverContent,
      },
      // Main content
      {
        properties: {
          page: {
            size: { width: A4_W, height: A4_H },
            margin: { top: MARGIN, right: MARGIN, bottom: MARGIN, left: MARGIN },
          },
        },
        headers: {
          default: new Header({
            children: [new Paragraph({
              children: [
                new TextRun({ text: "OpenSpatialDelay  \u2014  Third-Party Notices", font: FONT_BODY, size: 16, color: C.dimText }),
              ],
              border: { bottom: { style: BorderStyle.SINGLE, size: 4, color: C.navy, space: 4 } },
            })],
          }),
        },
        footers: {
          default: new Footer({
            children: [new Paragraph({
              children: [
                new TextRun({ text: "spatialmedialab.org", font: FONT_BODY, size: 16, color: C.dimText }),
                new TextRun({ text: "\t" }),
                new TextRun({ text: "Page ", font: FONT_BODY, size: 16, color: C.dimText }),
                new TextRun({ children: [PageNumber.CURRENT], font: FONT_BODY, size: 16, color: C.dimText }),
              ],
              tabStops: [{ type: TabStopType.RIGHT, position: TabStopPosition.MAX }],
            })],
          }),
        },
        children: mainContent,
      },
    ],
  });

  const buffer = await Packer.toBuffer(doc);
  fs.writeFileSync(OUTPUT_PATH, buffer);
  console.log(`Legal notices generated: ${OUTPUT_PATH}`);
  console.log(`File size: ${(buffer.length / 1024).toFixed(1)} KB`);
}

buildDocument().catch(err => {
  console.error("Error generating legal notices:", err);
  process.exit(1);
});
