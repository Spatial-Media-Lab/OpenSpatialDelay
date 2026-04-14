#!/usr/bin/env node
/**
 * OpenSpatialDelay v1.0 — Interactive PDF User Manual Generator
 *
 * Generates a .docx user manual with:
 * - Branded cover page
 * - Clickable table of contents
 * - Internal cross-references (bookmarks)
 * - External hyperlinks
 * - Placeholder image boxes for screenshots
 * - Embedded signal flow diagram
 *
 * Usage: node generate_manual.js
 * Output: OpenSpatialDelay_Manual_v1.0.docx
 */

const fs = require("fs");
const path = require("path");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  ImageRun, Header, Footer, AlignmentType, LevelFormat,
  ExternalHyperlink, InternalHyperlink, Bookmark,
  TableOfContents, HeadingLevel, BorderStyle, WidthType,
  ShadingType, PageNumber, PageBreak, TabStopType, TabStopPosition,
  PositionalTab, PositionalTabAlignment, PositionalTabRelativeTo,
  PositionalTabLeader, SectionType,
} = require("docx");

// ============================================================================
// CONSTANTS
// ============================================================================

const DOCS_DIR = __dirname;
const ASSETS_DIR = path.join(DOCS_DIR, "assets");
const FONTS_DIR = path.join(DOCS_DIR, "..", "fonts");
const OUTPUT_PATH = path.join(DOCS_DIR, "OpenSpatialDelay_Manual_v1.0.docx");

// Colors (hex without #, for docx-js)
const C = {
  navy:       "001633",
  bodyText:   "1A1A2E",
  subheading: "334155",
  calloutBg:  "0A0D12",
  calloutTx:  "E1E5EA",
  cyan:       "80D8FF",
  violet:     "7457D1",
  amber:      "F0A646",
  rose:       "E467A6",
  green:      "3BCE6C",
  gold:       "E1C34B",
  tableHdr:   "001633",
  tableHdrTx: "FFFFFF",
  tableAlt:   "F8FAFC",
  tableBorder:"E2E8F0",
  placeholder:"F0F4F8",
  placeholderBorder: "CBD5E1",
  dimText:    "64748B",
  white:      "FFFFFF",
  link:       "2563EB",
};

// A4 page in DXA (1 inch = 1440 DXA)
const A4_W = 11906;
const A4_H = 16838;
const MARGIN = 1440; // 1 inch
const CONTENT_W = A4_W - 2 * MARGIN; // 9026 DXA

// Fonts
const FONT_BODY = "DM Sans";
const FONT_MONO = "JetBrains Mono";
const FONT_FALLBACK = "Arial";

// ============================================================================
// ASSET LOADING
// ============================================================================

function loadImage(filename) {
  const p = path.join(ASSETS_DIR, filename);
  if (fs.existsSync(p)) return fs.readFileSync(p);
  return null;
}

const signalFlowImg = loadImage("signal-flow.png");
const logoImg = loadImage("sml-logo.png");
const coverImg = loadImage("manual-cover.png");
const screenshotFullImg = loadImage("screenshot.png");
const screenshotAnnotatedImg = loadImage("screenshot_annotated.png");
const screenshotHeaderImg = loadImage("screenshot_header.png");
const screenshotSpatialImg = loadImage("screenshot_spatial_map.png");
const screenshotElevationImg = loadImage("screenshot_elevation_map.png");
const screenshotBottomImg = loadImage("screenshot_bottom_panel.png");

// ============================================================================
// SECTION NUMBERING
// ============================================================================

class SectionNumberer {
  constructor() { this.h1 = 0; this.h2 = 0; this.h3 = 0; }
  reset() { this.h1 = 0; this.h2 = 0; this.h3 = 0; }
  next(level) {
    if (level === 1) { this.h1++; this.h2 = 0; this.h3 = 0; return `${this.h1}`; }
    if (level === 2) { this.h2++; this.h3 = 0; return `${this.h1}.${this.h2}`; }
    if (level === 3) { this.h3++; return `${this.h1}.${this.h2}.${this.h3}`; }
  }
}
const sn = new SectionNumberer();

// ============================================================================
// HELPER: TABLE BORDERS
// ============================================================================

const thinBorder = { style: BorderStyle.SINGLE, size: 1, color: C.tableBorder };
const thickAccentBorder = (color) => ({ style: BorderStyle.SINGLE, size: 8, color });
const noBorder = { style: BorderStyle.NONE, size: 0, color: "FFFFFF" };
const dashedBorder = { style: BorderStyle.DASHED, size: 1, color: C.placeholderBorder };
const allThinBorders = { top: thinBorder, bottom: thinBorder, left: thinBorder, right: thinBorder };
const allDashedBorders = { top: dashedBorder, bottom: dashedBorder, left: dashedBorder, right: dashedBorder };
const noBorders = { top: noBorder, bottom: noBorder, left: noBorder, right: noBorder };

// ============================================================================
// HELPER: STYLED TEXT RUNS
// ============================================================================

function bodyText(text, opts = {}) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color: C.bodyText, ...opts }); // 11pt
}

function boldText(text, opts = {}) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color: C.bodyText, bold: true, ...opts });
}

function monoText(text, opts = {}) {
  return new TextRun({ text, font: FONT_MONO, size: 18, color: C.subheading, ...opts }); // 9pt
}

function accentText(text, color, opts = {}) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color, bold: true, ...opts });
}

function linkText(text) {
  return new TextRun({ text, font: FONT_BODY, size: 22, color: C.link, underline: {} });
}

function dimText(text) {
  return new TextRun({ text, font: FONT_BODY, size: 20, color: C.dimText, italics: true });
}

// ============================================================================
// HELPER: PARAGRAPHS
// ============================================================================

function bodyPara(children, opts = {}) {
  const runs = typeof children === "string" ? [bodyText(children)] : children;
  return new Paragraph({
    spacing: { after: 160, line: 300 },
    ...opts,
    children: runs,
  });
}

function heading1(text, bookmarkId) {
  const num = sn.next(1);
  const displayText = `${num}  ${text}`;
  const children = bookmarkId
    ? [new Bookmark({ id: bookmarkId, children: [
        new TextRun({ text: `${num}`, bold: true, font: FONT_BODY, size: 36, color: C.dimText }),
        new TextRun({ text: `  ${text}`, bold: true, font: FONT_BODY, size: 36, color: C.navy }),
      ] })]
    : [
        new TextRun({ text: `${num}`, bold: true, font: FONT_BODY, size: 36, color: C.dimText }),
        new TextRun({ text: `  ${text}`, bold: true, font: FONT_BODY, size: 36, color: C.navy }),
      ];
  return new Paragraph({
    heading: HeadingLevel.HEADING_1,
    spacing: { before: 360, after: 240 },
    children,
  });
}

function heading2(text, bookmarkId) {
  const num = sn.next(2);
  const children = bookmarkId
    ? [new Bookmark({ id: bookmarkId, children: [
        new TextRun({ text: `${num}`, bold: true, font: FONT_BODY, size: 30, color: C.dimText }),
        new TextRun({ text: `  ${text}`, bold: true, font: FONT_BODY, size: 30, color: C.navy }),
      ] })]
    : [
        new TextRun({ text: `${num}`, bold: true, font: FONT_BODY, size: 30, color: C.dimText }),
        new TextRun({ text: `  ${text}`, bold: true, font: FONT_BODY, size: 30, color: C.navy }),
      ];
  return new Paragraph({
    heading: HeadingLevel.HEADING_2,
    spacing: { before: 280, after: 180 },
    children,
  });
}

function heading3(text) {
  const num = sn.next(3);
  return new Paragraph({
    heading: HeadingLevel.HEADING_3,
    spacing: { before: 200, after: 120 },
    children: [
      new TextRun({ text: `${num}`, bold: true, font: FONT_BODY, size: 26, color: C.dimText }),
      new TextRun({ text: `  ${text}`, bold: true, font: FONT_BODY, size: 26, color: C.subheading }),
    ],
  });
}

function spacer(pts = 12) {
  return new Paragraph({ spacing: { after: pts * 20 } });
}

function internalLink(text, anchor) {
  return new InternalHyperlink({
    anchor,
    children: [new TextRun({ text, font: FONT_BODY, size: 22, color: C.link, underline: {} })],
  });
}

function externalLink(text, url) {
  return new ExternalHyperlink({
    link: url,
    children: [linkText(text)],
  });
}

// ============================================================================
// HELPER: CALLOUT BOX (dark themed table cell with accent left border)
// ============================================================================

function calloutBox(title, contentParagraphs, accentColor = C.cyan) {
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
              left: thickAccentBorder(accentColor),
              right: { style: BorderStyle.SINGLE, size: 1, color: "2A2E35" },
            },
            shading: { fill: C.calloutBg, type: ShadingType.CLEAR },
            margins: { top: 120, bottom: 120, left: 200, right: 200 },
            width: { size: CONTENT_W, type: WidthType.DXA },
            children: [
              new Paragraph({
                spacing: { after: 100 },
                children: [new TextRun({ text: title, font: FONT_BODY, size: 22, color: accentColor, bold: true })],
              }),
              ...contentParagraphs.map(p =>
                typeof p === "string"
                  ? new Paragraph({
                      spacing: { after: 80, line: 280 },
                      children: [new TextRun({ text: p, font: FONT_BODY, size: 20, color: C.calloutTx })],
                    })
                  : p
              ),
            ],
          }),
        ],
      }),
    ],
  });
}

function calloutPara(children) {
  const runs = typeof children === "string"
    ? [new TextRun({ text: children, font: FONT_BODY, size: 20, color: C.calloutTx })]
    : children;
  return new Paragraph({ spacing: { after: 80, line: 280 }, children: runs });
}

// ============================================================================
// HELPER: EMBEDDED SCREENSHOT (or placeholder fallback)
// ============================================================================

function screenshotImage(imgData, widthPts, heightPts, altTitle, altDescription) {
  if (imgData) {
    return new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { before: 80, after: 120 },
      children: [new ImageRun({
        type: "png",
        data: imgData,
        transformation: { width: widthPts, height: heightPts },
        altText: { title: altTitle, description: altDescription, name: altTitle.toLowerCase().replace(/\s+/g, "-") },
      })],
    });
  }
  return placeholderBox(altDescription, heightPts);
}

function placeholderBox(description, heightPts = 200) {
  const spacingLines = Math.max(1, Math.floor(heightPts / 14));
  const padParagraphs = [];
  for (let i = 0; i < Math.floor(spacingLines / 2) - 1; i++) {
    padParagraphs.push(new Paragraph({ spacing: { after: 0 } }));
  }

  return new Table({
    width: { size: CONTENT_W, type: WidthType.DXA },
    columnWidths: [CONTENT_W],
    rows: [
      new TableRow({
        children: [
          new TableCell({
            borders: allDashedBorders,
            shading: { fill: C.placeholder, type: ShadingType.CLEAR },
            margins: { top: 80, bottom: 80, left: 120, right: 120 },
            width: { size: CONTENT_W, type: WidthType.DXA },
            children: [
              ...padParagraphs,
              new Paragraph({
                alignment: AlignmentType.CENTER,
                spacing: { after: 40 },
                children: [dimText(`[ Screenshot: ${description} ]`)],
              }),
              ...padParagraphs,
            ],
          }),
        ],
      }),
    ],
  });
}

// ============================================================================
// HELPER: DATA TABLE
// ============================================================================

function dataTable(headers, rows, opts = {}) {
  const numCols = headers.length;
  const colWidths = opts.colWidths || headers.map(() => Math.floor(CONTENT_W / numCols));
  // Ensure colWidths sum to CONTENT_W
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

// ============================================================================
// SECTION: COVER PAGE
// ============================================================================

function buildCover() {
  const children = [];

  // Full-page cover image (Orbital Cartography design)
  if (coverImg) {
    children.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      children: [new ImageRun({
        type: "png",
        data: coverImg,
        transformation: { width: 595, height: 842 },  // A4 proportions in points
        altText: { title: "OpenSpatialDelay v1.0 User Manual Cover", description: "Cover page showing spatial map with 12 delay taps positioned in 3D space", name: "manual-cover" },
      })],
    }));
  } else {
    // Fallback: text-based cover
    children.push(spacer(40));
    if (logoImg) {
      children.push(new Paragraph({
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
    children.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 80 },
      children: [new TextRun({ text: "OpenSpatialDelay", font: FONT_BODY, size: 72, color: C.navy, bold: true })],
    }));
    children.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 140 },
      children: [new TextRun({ text: "v1.0  User Manual", font: FONT_BODY, size: 32, color: C.subheading })],
    }));
    children.push(screenshotImage(screenshotFullImg, 420, 297,
      "OpenSpatialDelay Interface",
      "Full plugin UI showing spatial map, controls, and tap selector"));
    children.push(spacer(20));
    children.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      children: [
        externalLink("spatialmedialab.org", "https://spatialmedialab.org"),
        new TextRun({ text: "  |  2026", font: FONT_BODY, size: 20, color: C.dimText }),
      ],
    }));
  }

  return children;
}

// ============================================================================
// SECTION: TABLE OF CONTENTS
// ============================================================================

function tocEntry(text, anchor, level = 1) {
  const num = sn.next(level);
  const indent = level === 1 ? 0 : level === 2 ? 400 : 800;
  const fontSize = level === 1 ? 24 : 21;
  const isBold = level === 1;
  return new Paragraph({
    spacing: { after: level === 1 ? 140 : 80, before: level === 1 ? 80 : 0 },
    indent: { left: indent },
    children: [
      new InternalHyperlink({
        anchor,
        children: [
          new TextRun({ text: `${num}`, font: FONT_BODY, size: fontSize, color: C.dimText, bold: isBold }),
          new TextRun({ text: `  ${text}`, font: FONT_BODY, size: fontSize, color: C.navy, bold: isBold }),
        ],
      }),
    ],
  });
}

function buildTOC() {
  sn.reset();
  const entries = [
    new Paragraph({
      spacing: { before: 200, after: 400 },
      children: [new TextRun({ text: "Contents", font: FONT_BODY, size: 40, color: C.navy, bold: true })],
    }),
    // 1 Quick Start
    tocEntry("Quick Start", "quick-start"),
    tocEntry("Installation", "installation", 2),
    tocEntry("Load in Your DAW", "load-in-daw", 2),
    tocEntry("Interface Overview", "interface-overview", 2),
    tocEntry("Signal Flow", "signal-flow", 2),
    tocEntry("Format Quick Pick", "format-quick-pick", 2),

    // 2 What is OpenSpatialDelay?
    tocEntry("What is OpenSpatialDelay?", "what-is"),
    tocEntry("What Makes It Unique", "unique", 2),

    // 3 The Spatial Map
    tocEntry("The Spatial Map", "spatial-map"),
    tocEntry("Coordinate System", "coordinate-system", 2),
    tocEntry("Interaction", "map-interaction", 2),
    tocEntry("Elevation Visualization", "elevation-viz", 2),

    // 4 Output Formats & Algorithms
    tocEntry("Output Formats & Algorithms", "output-formats"),
    tocEntry("Five Rendering Paths", "rendering-paths", 2),
    tocEntry("Binaural & Stereo Formats", "binaural-stereo", 2),
    tocEntry("Binaural (2 channels)", "binaural-stereo", 3),
    tocEntry("Stereo (2 channels, 5 modes)", "binaural-stereo", 3),
    tocEntry("Surround Formats", "surround-formats", 2),
    tocEntry("Ambisonics Formats", "ambisonics-formats", 2),
    tocEntry("Spatialization Algorithms", "algorithms", 2),
    tocEntry("HRTF Profiles", "hrtf-profiles", 2),

    // 5 Controls Reference
    tocEntry("Controls Reference", "controls-reference"),
    tocEntry("Header Bar", "header-bar", 2),
    tocEntry("DELAY Section", "delay-section", 2),
    tocEntry("MOD Section", "mod-section", 2),
    tocEntry("TONE Section", "tone-section", 2),
    tocEntry("MIX Section", "mix-section", 2),
    tocEntry("OSC Section", "osc-section", 2),
    tocEntry("Per-Tap Controls", "per-tap-controls", 2),

    // 6–10
    tocEntry("Trajectories & Animation", "trajectories"),
    tocEntry("Creative Tips", "creative-tips"),
    tocEntry("Presets", "presets"),
    tocEntry("Preset File Locations", "preset-locations", 2),
    tocEntry("Saving Presets", "saving-presets", 2),
    tocEntry("ADM-OSC Integration", "adm-osc"),
    tocEntry("Receive (Incoming Control)", "adm-osc", 2),
    tocEntry("Send (Broadcast Positions)", "adm-osc", 2),
    tocEntry("Troubleshooting", "troubleshooting"),
    tocEntry("Third-Party Notices", "legal-notices"),
  ];
  return entries;
}

// ============================================================================
// SECTION 1: QUICK START
// ============================================================================

function buildQuickStart() {
  const items = [];
  items.push(heading1("Quick Start", "quick-start"));
  items.push(bodyPara([
    bodyText("This page gets you from installation to first sound in under two minutes. For detailed explanations, see the sections that follow."),
  ]));

  // Installation
  items.push(heading2("Installation", "installation"));
  items.push(bodyPara([
    bodyText("Download the latest release for your platform from the "),
    externalLink("GitHub Releases page", "https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases"),
    bodyText("."),
  ]));
  items.push(bodyPara([
    boldText("macOS: "),
    bodyText("Run the .pkg installer. It places the AU and VST3 plugins in "),
    monoText("~/Library/Audio/Plug-Ins/"),
    bodyText(" automatically."),
  ]));
  items.push(bodyPara([
    boldText("Windows: "),
    bodyText("Run the .exe installer. The VST3 is installed to "),
    monoText("C:\\Program Files\\Common Files\\VST3\\"),
    bodyText("."),
  ]));

  // Load in DAW
  items.push(heading2("Load in Your DAW", "load-in-daw"));
  items.push(bodyPara("Insert OpenSpatialDelay as an effect (insert/send) on any audio or bus track. For binaural (headphone) output, use a stereo track. For surround formats, match the track channel count to your target format."));

  // UI Overview
  items.push(heading2("Interface Overview", "interface-overview"));
  items.push(screenshotImage(screenshotAnnotatedImg, 480, 340,
    "Plugin Interface Overview",
    "Full Plugin UI \u2014 820\u00d7580 window with four labeled regions"));
  items.push(spacer(8));
  items.push(bodyPara([bodyText("The interface has four regions:")]));
  items.push(bodyPara([boldText("A) Header Bar "), bodyText("(top) \u2014 presets, output format, and algorithm selection.")]));
  items.push(bodyPara([boldText("B) Spatial Map "), bodyText("(left) \u2014 top-down view of 3D space where you position delay taps.")]));
  items.push(bodyPara([boldText("C) Bottom Panel "), bodyText("(bottom-left) \u2014 per-tap controls for the selected object.")]));
  items.push(bodyPara([boldText("D) Right Panel "), bodyText("(right) \u2014 global controls for delay, tone, modulation, and mix.")]));


  // Signal flow
  items.push(heading2("Signal Flow", "signal-flow"));
  if (signalFlowImg) {
    items.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 120 },
      children: [new ImageRun({
        type: "png",
        data: signalFlowImg,
        transformation: { width: 480, height: 180 },
        altText: { title: "Signal Flow", description: "5-stage per-sample signal flow diagram", name: "signal-flow" },
      })],
    }));
  }
  items.push(bodyPara([
    bodyText("Audio flows through five stages: "),
    boldText("Input "),
    bodyText("is mixed with feedback and written to the delay buffer. Each enabled "),
    boldText("tap "),
    bodyText("reads from the buffer at its sequential position and applies pitch shifting. Each tap is then "),
    boldText("spatialized "),
    bodyText("to its 3D position. The "),
    boldText("feedback "),
    bodyText("loop reads mono from the end of the tap chain, filtering and soft-clipping before feeding back. Finally, dry and wet signals are mixed at the "),
    boldText("output"),
    bodyText(". See "),
    internalLink("Controls Reference", "controls-reference"),
    bodyText(" for parameter details."),
  ]));

  // Quick pick table
  items.push(heading2("Format Quick Pick", "format-quick-pick"));
  items.push(bodyPara("Choose your output format based on your workflow:"));
  items.push(dataTable(
    ["Goal", "Output Format", "Algorithm / Profile"],
    [
      ["Headphone mixing", "Binaural", "Select HRTF profile (try Studio Reference)"],
      ["Stereo monitoring", "Stereo", "Choose mic mode (Equal Power, XY, MS, etc.)"],
      ["Dolby Atmos (7.1.4)", "7.1.4 Atmos", "VBAP or MDAP"],
      ["Ambisonics pipeline", "FOA through 6OA", "Ambisonics Encode (automatic)"],
      ["Standard surround", "5.1 / 7.1", "VBAP (focused) or DBAP (diffuse)"],
    ],
    { colWidths: [2200, 2600, 4226] }
  ));

  return items;
}

// ============================================================================
// SECTION 2: WHAT IS OPENSPATIALDELAY?
// ============================================================================

function buildWhatIs() {
  const items = [];
  items.push(heading1("What is OpenSpatialDelay?", "what-is"));

  items.push(bodyPara([
    bodyText("OpenSpatialDelay is a "),
    boldText("spatial delay effect "),
    bodyText("where each delay tap is positioned in 3D space. Think of a classic stereo ping-pong delay \u2014 but instead of bouncing between left and right, the echoes travel through up to 12 spatial positions around the listener."),
  ]));

  items.push(bodyPara("Each tap reads from a shared delay line at sequential intervals: Tap 1 plays at 1\u00d7 the delay time, Tap 2 at 2\u00d7, and so on. Every tap has its own position (azimuth, elevation, distance), trajectory animation, pitch shift, and Doppler amount."));

  items.push(bodyPara([
    bodyText("The output is rendered through one of "),
    boldText("five rendering paths"),
    bodyText(": direct binaural HRTF convolution for headphones, stereo mic simulation, discrete surround speaker panning, or Ambisonics spherical harmonics encoding. See "),
    internalLink("Output Formats & Algorithms", "output-formats"),
    bodyText(" for full details."),
  ]));

  // Terminology callout
  items.push(spacer(8));
  items.push(calloutBox("Key Terminology", [
    calloutPara([
      new TextRun({ text: "Azimuth: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "Horizontal angle around the listener. 0\u00b0 = directly in front, +90\u00b0 = left, \u221290\u00b0 = right, \u00b1180\u00b0 = behind.", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
    calloutPara([
      new TextRun({ text: "Elevation: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "Vertical angle. 0\u00b0 = ear level (horizon), +90\u00b0 = directly above, \u221290\u00b0 = directly below.", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
    calloutPara([
      new TextRun({ text: "Distance: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "How far the sound source is from the listener (0.1 = very close, 3.0 = far away). Affects volume attenuation and air absorption.", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
    calloutPara([
      new TextRun({ text: "HRTF: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "Head-Related Transfer Function \u2014 measurements of how sound changes as it travels around your head and ears. Used to create convincing 3D audio over headphones.", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
    calloutPara([
      new TextRun({ text: "Binaural: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "Audio rendered for headphone listening, using HRTF processing to simulate 3D positioning.", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
    calloutPara([
      new TextRun({ text: "Surround: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "Audio rendered to discrete speaker channels (5.1, 7.1.4 Atmos, etc.).", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
    calloutPara([
      new TextRun({ text: "Ambisonics: ", font: FONT_BODY, size: 20, color: C.cyan, bold: true }),
      new TextRun({ text: "A format for encoding 3D audio as spherical harmonics. Flexible \u2014 can be decoded to any speaker layout later.", font: FONT_BODY, size: 20, color: C.calloutTx }),
    ]),
  ]));
  items.push(spacer(12));

  // What makes it unique
  items.push(heading2("What Makes It Unique", "unique"));
  items.push(bodyPara([
    boldText("12 independent spatial taps "),
    bodyText("\u2014 each positioned anywhere in 3D space, with its own trajectory animation, pitch shift, Doppler amount, and input channel selection."),
  ]));
  items.push(bodyPara([
    boldText("23 output formats "),
    bodyText("\u2014 from binaural headphones to 9.1.6 Dolby Atmos, SpatialMediaLab 13.1, and 6th-order Ambisonics."),
  ]));
  items.push(bodyPara([
    boldText("8 spatialization algorithms "),
    bodyText("\u2014 Constant Power, VBAP, VBIP, MDAP, KNN, DBAP, Ambisonics, and direct binaural HRTF convolution with 6 measured profiles."),
  ]));
  items.push(bodyPara([
    boldText("Open source "),
    bodyText("\u2014 part of the Spatial Media Library suite. Free to use and modify."),
  ]));

  return items;
}

// ============================================================================
// SECTION 3: THE SPATIAL MAP (DEEP)
// ============================================================================

function buildSpatialMap() {
  const items = [];
  items.push(heading1("The Spatial Map", "spatial-map"));

  items.push(bodyPara("The spatial map is the large visualization on the left side of the plugin window. It shows a top-down view of 3D space with the listener at the center. Each colored dot represents a delay tap positioned in space."));

  items.push(screenshotImage(screenshotSpatialImg, 380, 280,
    "Spatial Map",
    "Spatial Map Close-Up \u2014 showing taps positioned around the listener with distance rings and compass labels"));
  items.push(spacer(8));

  // Coordinate system
  items.push(heading2("Coordinate System", "coordinate-system"));
  items.push(bodyPara([
    bodyText("The map uses a "),
    boldText("polar coordinate system"),
    bodyText(". The listener sits at the center. Front is at the "),
    boldText("top"),
    bodyText(" of the map (marked F). Concentric rings represent distance from the listener."),
  ]));

  items.push(dataTable(
    ["Direction", "Azimuth", "Position on Map"],
    [
      ["Front", "0\u00b0", "Top"],
      ["Left", "+90\u00b0", "Left"],
      ["Rear", "\u00b1180\u00b0", "Bottom"],
      ["Right", "\u221290\u00b0", "Right"],
    ],
    { colWidths: [2500, 2500, 4026] }
  ));
  items.push(spacer(8));

  items.push(bodyPara([
    bodyText("Note: The azimuth knob uses "),
    boldText("reverse rotation"),
    bodyText(" \u2014 turning the knob clockwise moves the tap clockwise on the map. This follows the IEM StereoEncoder convention used in professional spatial audio tools."),
  ]));

  // Interaction
  items.push(heading2("Interaction", "map-interaction"));
  items.push(bodyPara([
    boldText("Click "),
    bodyText("on a tap dot to select it. The selected tap is highlighted with a glowing ring, and the bottom panel updates to show that tap's controls."),
  ]));
  items.push(bodyPara([
    boldText("Click and drag "),
    bodyText("a tap to move it in azimuth (rotation) and distance (radial position) simultaneously."),
  ]));
  items.push(bodyPara([
    bodyText("Elevation is controlled via the "),
    boldText("ELEV knob"),
    bodyText(" in the bottom panel, not by map interaction."),
  ]));

  // Elevation visualization
  items.push(heading2("Elevation Visualization", "elevation-viz"));
  items.push(bodyPara("Although the map is 2D (top-down), elevation is communicated visually through the dot appearance:"));
  items.push(bodyPara([
    boldText("Upper hemisphere "),
    bodyText("(elevation > 0\u00b0): Tap dots are solid and opaque, slightly larger (+5px). These represent sounds above ear level."),
  ]));
  items.push(bodyPara([
    boldText("Lower hemisphere "),
    bodyText("(elevation < 0\u00b0): Tap dots become transparent (30% opacity) and slightly smaller (\u22123px). Minimum dot size is 11px at \u221290\u00b0."),
  ]));
  items.push(bodyPara("At 0\u00b0 elevation (ear level), dots are at their default size and full opacity."));

  items.push(spacer(8));
  items.push(screenshotImage(screenshotElevationImg, 380, 280,
    "Elevation Visualization",
    "Spatial Map with multiple taps at various positions, showing elevation transparency and color coding"));

  return items;
}

// ============================================================================
// SECTION 4: OUTPUT FORMATS & ALGORITHMS (DEEP)
// ============================================================================

function buildOutputFormats() {
  const items = [];
  items.push(heading1("Output Formats & Algorithms", "output-formats"));

  items.push(bodyPara([
    bodyText("OpenSpatialDelay supports "),
    boldText("23 output formats"),
    bodyText(" organized into four categories. The plugin automatically selects the correct rendering path based on the format you choose in the header dropdown."),
  ]));

  // Rendering paths
  items.push(heading2("Five Rendering Paths", "rendering-paths"));
  items.push(bodyPara("The plugin dispatches audio through one of five peer rendering paths based on the selected output format:"));

  items.push(dataTable(
    ["Rendering Path", "When Used", "Output"],
    [
      ["Direct Binaural HRTF", "Binaural format + SOFA profile", "2 ch (L/R)"],
      ["Simple Binaural (Woodworth)", "Binaural + Simple profile selected", "2 ch (L/R)"],
      ["Stereo Variants", "Stereo format", "2 ch (L/R)"],
      ["Ambisonics Output", "Any Ambisonics format (FOA\u20136OA)", "4\u201349 ch"],
      ["Discrete Surround", "Any surround format (Quad\u20139.1.6)", "4\u201316 ch"],
    ],
    { colWidths: [2800, 3400, 2826] }
  ));
  items.push(spacer(8));

  // Binaural + Stereo formats
  items.push(heading2("Binaural & Stereo Formats", "binaural-stereo"));

  items.push(heading3("Binaural (2 channels)"));
  items.push(bodyPara([
    bodyText("The default format. Each tap's 3D position is rendered through "),
    boldText("per-source HRTF convolution "),
    bodyText("\u2014 a separate convolver for each active tap, using measured head-related transfer functions from SOFA files (Spatially Oriented Format for Acoustics). This produces highly realistic 3D audio over headphones. Select a profile from the "),
    boldText("HRTF Profile"),
    bodyText(" dropdown (see "),
    internalLink("HRTF Profiles", "hrtf-profiles"),
    bodyText(")."),
  ]));

  items.push(heading3("Stereo (2 channels, 5 modes)"));
  items.push(bodyPara("When Stereo is selected, the Algorithm dropdown switches to offer five microphone simulation modes:"));

  items.push(dataTable(
    ["Mode", "Description"],
    [
      ["Equal Power Pan", "Standard cosine/sine pan law from azimuth angle"],
      ["VBAP (2-speaker)", "Virtual speakers at \u00b130\u00b0, VBAP gain computation"],
      ["XY Pair", "Coincident cardioid microphones at \u00b145\u00b0"],
      ["MS (Mid-Side)", "Mid = cos(azimuth), Side = sin(azimuth)"],
      ["Blumlein", "Crossed figure-8 microphones at \u00b145\u00b0"],
    ],
    { colWidths: [2500, 6526] }
  ));
  items.push(spacer(8));

  // Surround formats
  items.push(heading2("Surround Formats", "surround-formats"));
  items.push(bodyPara([
    bodyText("15 discrete surround formats, from Quadraphonic to 9.1.6 Atmos and SpatialMediaLab 13.1. Formats with an LFE channel (marked with .1) derive it as a mono sum of all tap outputs through a 120 Hz low-pass filter at \u221210 dB. Choose a "),
    internalLink("spatialization algorithm", "algorithms"),
    bodyText(" from the Algorithm dropdown."),
  ]));

  items.push(dataTable(
    ["Format", "Channels", "LFE", "Height Speakers"],
    [
      ["Quadraphonic", "4", "No", "None"],
      ["5.0 Surround", "5", "No", "None"],
      ["5.1 Surround", "6", "Yes", "None"],
      ["7.0 Surround", "7", "No", "None"],
      ["7.1 Surround", "8", "Yes", "None"],
      ["9.1 Surround", "10", "Yes", "None (9 ear-level, ITU-R BS.2051 System H)"],
      ["Octaphonic", "8", "No", "None (ring at 45\u00b0 intervals)"],
      ["5.1.2", "8", "Yes", "2 top"],
      ["7.1.2 Atmos", "10", "Yes", "2 top"],
      ["5.1.4 Atmos", "10", "Yes", "4 top"],
      ["7.1.4 Atmos", "12", "Yes", "4 top"],
      ["7.1.6 Atmos", "14", "Yes", "6 top"],
      ["9.1.4 Atmos", "14", "Yes", "4 top + wide speakers"],
      ["9.1.6 Atmos", "16", "Yes", "6 top + wide speakers"],
      ["SpatialMediaLab 13.1", "14", "Yes", "8 ear-level + 4 height + 1 zenith"],
    ],
    { colWidths: [2400, 1400, 1000, 4226] }
  ));
  items.push(spacer(8));

  // Ambisonics
  items.push(heading2("Ambisonics Formats", "ambisonics-formats"));
  items.push(bodyPara("Six Ambisonics orders using ACN/SN3D (AmbiX) channel ordering. Decoded to speakers by your DAW or external renderer."));

  items.push(dataTable(
    ["Format", "Order", "Channels"],
    [
      ["FOA (1st Order)", "1", "4"],
      ["SOA (2nd Order)", "2", "9"],
      ["HOA (3rd Order)", "3", "16"],
      ["4OA (4th Order)", "4", "25"],
      ["5OA (5th Order)", "5", "36"],
      ["6OA (6th Order)", "6", "49"],
    ],
    { colWidths: [3200, 2000, 3826] }
  ));
  items.push(spacer(8));

  // Algorithms
  items.push(heading2("Spatialization Algorithms", "algorithms"));
  items.push(bodyPara([
    bodyText("When using surround output formats, the "),
    boldText("Algorithm"),
    bodyText(" dropdown lets you choose how tap positions are translated to speaker gains:"),
  ]));

  items.push(dataTable(
    ["Algorithm", "Best For", "Character"],
    [
      ["Constant Power", "Most surround work (default)", "Cosine-distance weighting, wide natural rolloff"],
      ["VBAP", "Precise point sources, production mixing", "Focused, sharp localization"],
      ["VBIP", "Off-axis listener stability", "VBAP with intensity weighting, smoother"],
      ["MDAP", "Wide, stable spatial images", "VBAP + 8 spread sub-sources around main position"],
      ["KNN", "Irregular or curved speaker arrays", "3 nearest speakers, distance-weighted, natural"],
      ["DBAP", "Non-standard speaker layouts", "Distance-based, warm and diffuse"],
      ["Ambisonics", "Ambisonics workflows, high-order setups", "SH encode/decode, smooth panning"],
    ],
    { colWidths: [1800, 3200, 4026] }
  ));
  items.push(spacer(8));

  items.push(calloutBox("Which algorithm should I use?", [
    "Start with Constant Power (the default) \u2014 it provides smooth, natural panning suitable for most surround work.",
    "For sharper, more focused point sources, switch to VBAP. For wider, more stable sources, try MDAP. For non-standard or irregular speaker arrangements, use DBAP.",
    "The algorithm dropdown only appears for surround formats. For Ambisonics output, the algorithm is always Ambisonics. For binaural output, HRTF convolution handles spatialization directly.",
  ], C.cyan));
  items.push(spacer(8));

  // HRTF Profiles
  items.push(heading2("HRTF Profiles", "hrtf-profiles"));
  items.push(bodyPara([
    bodyText("When Binaural output is selected, the "),
    boldText("HRTF Profile"),
    bodyText(" dropdown offers six options. Each profile uses different measured head-related transfer functions, producing a subtly different spatial sound:"),
  ]));

  items.push(dataTable(
    ["Profile", "Source", "Character", "CPU"],
    [
      ["Simple (Low CPU)", "Woodworth head model", "No convolution \u2014 ITD+ILD only, lowest latency", "Minimal"],
      ["Immersive", "SADIE II D2 KU100", "Rich spatial detail, strong elevation cues", "Normal"],
      ["Natural", "CIPIC Subject 003", "Organic, realistic binaural rendering", "Normal"],
      ["Precise", "HUTUBS PP2", "Detailed, analytical spatial accuracy", "Normal"],
      ["Spatial", "Bernschuetz KU100", "Dense full-sphere measurement, widest coverage", "Normal"],
      ["Studio Reference", "MIT KEMAR Large Pinna", "Neutral, classic reference standard", "Normal"],
    ],
    { colWidths: [1800, 2200, 3226, 1800] }
  ));
  items.push(spacer(8));

  items.push(calloutBox("Choosing a profile", [
    "Start with Studio Reference for mixing. Switch to Simple if you need lower CPU usage.",
    "Different profiles suit different head shapes. Try each to find the one that sounds most natural to you \u2014 what works best is personal.",
  ], C.violet));
  items.push(spacer(8));

  items.push(bodyPara([
    boldText("Low-frequency bypass: "),
    bodyText("Some HRTF measurements lack low-frequency content below 200 Hz. OpenSpatialDelay automatically detects this and routes bass directly to the output, bypassing the HRTF convolution. This ensures solid bass regardless of which HRTF profile you choose."),
  ]));

  return items;
}

// ============================================================================
// SECTION 5: CONTROLS REFERENCE
// ============================================================================

function buildControlsReference() {
  const items = [];
  items.push(heading1("Controls Reference", "controls-reference"));

  // Header bar
  items.push(heading2("Header Bar", "header-bar"));
  items.push(screenshotImage(screenshotHeaderImg, 480, 30,
    "Header Bar",
    "Header Bar \u2014 showing title, preset navigation, OSC toggle, Output Format, Algorithm/HRTF dropdown"));
  items.push(spacer(4));
  items.push(bodyPara([
    bodyText("The header bar contains: plugin title and version (left), "),
    boldText("preset navigation "),
    bodyText("(dropdown, prev/next arrows, save button), "),
    boldText("OSC toggle "),
    bodyText("(ADM-OSC receive on/off), "),
    boldText("Output Format "),
    bodyText("dropdown, and either "),
    boldText("Algorithm "),
    bodyText("(surround) or "),
    boldText("HRTF Profile "),
    bodyText("(binaural) dropdown, which swap automatically based on the selected format."),
  ]));

  // Right panel - DELAY
  items.push(heading2("DELAY Section", "delay-section"));
  items.push(bodyPara([
    bodyText("Controls the core delay timing and feedback. Accent color: "),
    accentText("cyan", C.cyan),
    bodyText("."),
  ]));

  items.push(dataTable(
    ["Parameter", "Range", "Default", "Description"],
    [
      ["TIME", "1\u20132000 ms", "500 ms", "Base delay time. Tap k reads at k \u00d7 TIME."],
      ["SYNC", "On / Off", "Off", "Locks delay time to DAW tempo"],
      ["NOTE", "1/32 \u2013 2/1", "1/4", "Note division (when SYNC is on)"],
      ["MODE", "Notes, Dotted, Triplet", "Notes", "Sync modifier (when SYNC is on)"],
      ["FEEDBACK", "0\u2013100%", "30%", "Amount of delay output fed back to input"],
    ],
    { colWidths: [1500, 2200, 1200, 4126] }
  ));
  items.push(spacer(6));

  // MOD section
  items.push(heading2("MOD Section", "mod-section"));
  items.push(bodyPara([
    bodyText("Delay-time LFO modulation (wobble). Accent color: "),
    accentText("rose", C.rose),
    bodyText("."),
  ]));

  items.push(dataTable(
    ["Parameter", "Range", "Default", "Description"],
    [
      ["MOD toggle", "On / Off", "Off", "Enable wobble modulation"],
      ["AMOUNT", "0\u2013100%", "0%", "LFO modulation depth (milliseconds)"],
      ["MORPH", "0\u2013100%", "0%", "Waveform shape: sine \u2192 triangle \u2192 square"],
    ],
    { colWidths: [1500, 2200, 1200, 4126] }
  ));
  items.push(spacer(6));

  // TONE section
  items.push(heading2("TONE Section", "tone-section"));
  items.push(bodyPara([
    bodyText("Feedback path filters and air absorption. Accent color: "),
    accentText("violet", C.violet),
    bodyText(". Filters apply to the feedback path only \u2014 the first echo is always unfiltered."),
  ]));

  items.push(dataTable(
    ["Parameter", "Range", "Default", "Description"],
    [
      ["FLT toggle", "On / Off", "Off", "Enable feedback path filters"],
      ["HP", "20\u20135000 Hz", "50 Hz", "High-pass cutoff frequency"],
      ["LP", "200\u201320000 Hz", "5000 Hz", "Low-pass cutoff frequency"],
      ["HP RES", "0.1\u20138.0", "0.71", "High-pass filter resonance (Q)"],
      ["LP RES", "0.1\u20138.0", "0.71", "Low-pass filter resonance (Q)"],
      ["AIR toggle", "On / Off", "Off", "Distance-based HF rolloff per tap"],
    ],
    { colWidths: [1500, 2200, 1200, 4126] }
  ));
  items.push(spacer(6));

  // MIX section
  items.push(heading2("MIX Section", "mix-section"));
  items.push(bodyPara([
    bodyText("Output mixing and gain staging. Accent color: "),
    accentText("amber", C.amber),
    bodyText("."),
  ]));

  items.push(dataTable(
    ["Parameter", "Range", "Default", "Description"],
    [
      ["DRY/WET", "0\u2013100%", "50%", "Blend between dry input and processed output"],
      ["IN", "\u221296 to +24 dB", "0 dB", "Input gain (wet path only, not dry signal)"],
      ["OUT", "\u221296 to +24 dB", "0 dB", "Output gain (applied to both dry and wet)"],
    ],
    { colWidths: [1500, 2200, 1200, 4126] }
  ));
  items.push(spacer(6));

  // OSC section (brief)
  items.push(heading2("OSC Section", "osc-section"));
  items.push(bodyPara([
    bodyText("ADM-OSC receive and send controls. Accent color: "),
    accentText("green", C.green),
    bodyText(". See "),
    internalLink("ADM-OSC", "adm-osc"),
    bodyText(" for protocol details."),
  ]));
  items.push(dataTable(
    ["Parameter", "Default", "Description"],
    [
      ["RECV toggle", "Off", "Enable ADM-OSC receive (incoming position control)"],
      ["Port", "4002", "UDP port for incoming OSC messages"],
      ["SEND toggle", "Off", "Enable ADM-OSC send (broadcast positions)"],
      ["IP", "127.0.0.1", "Target IP for OSC send"],
      ["Send Port", "4003", "Target port for OSC send"],
    ],
    { colWidths: [1800, 1500, 5726] }
  ));
  items.push(spacer(6));

  // Bottom panel - Per-tap
  items.push(heading2("Per-Tap Controls (Bottom Panel)", "per-tap-controls"));
  items.push(screenshotImage(screenshotBottomImg, 480, 70,
    "Bottom Panel",
    "Bottom Panel \u2014 showing tap selector buttons (1\u201312) and per-tap controls"));
  items.push(spacer(4));
  items.push(bodyPara("Select a tap by clicking its numbered button (1\u201312). The controls below update to show that tap's settings:"));

  items.push(dataTable(
    ["Parameter", "Range", "Default", "Description"],
    [
      ["ON/OFF", "Toggle", "Taps 1\u20134: On", "Enable/disable this tap"],
      ["AZIMUTH", "\u2212180 to +180\u00b0", "Varies", "Horizontal angle (clockwise knob = clockwise map)"],
      ["ELEV", "\u221290 to +90\u00b0", "0\u00b0", "Vertical angle"],
      ["DIST", "0.1\u20133.0", "0.5", "Distance from listener (affects attenuation)"],
      ["DOPPLER", "0\u2013100%", "0%", "Doppler effect intensity for moving taps"],
      ["PITCH", "\u221212 to +12 st", "0 st", "Per-tap pitch offset (adds to global pitch)"],
      ["INPUT", "L+R / L / R", "L+R", "Which stereo input channel feeds this tap"],
      ["TRAJ", "14 shapes", "None", "Trajectory animation shape"],
      ["SPEED", "0.00\u20135.00 Hz", "0.30 Hz", "Trajectory animation speed"],
      ["DIR", "Fwd / Rev", "Fwd", "Trajectory direction (forward or reverse)"],
    ],
    { colWidths: [1500, 2200, 1400, 3926] }
  ));

  return items;
}

// ============================================================================
// SECTION 6: TRAJECTORIES & ANIMATION
// ============================================================================

function buildTrajectories() {
  const items = [];
  items.push(heading1("Trajectories & Animation", "trajectories"));

  items.push(bodyPara("Each tap can follow an animated trajectory through 3D space. Select a shape from the TRAJ dropdown in the bottom panel."));

  items.push(bodyPara([
    boldText("Origin-point architecture: "),
    bodyText("The knob values (azimuth, elevation, distance) set the tap's origin point. The trajectory animates around this origin. You can reposition a running trajectory by adjusting the knobs \u2014 the animation follows. A crosshair marker on the spatial map shows the origin position."),
  ]));

  items.push(dataTable(
    ["Shape", "Motion", "Best For"],
    [
      ["None", "Static position, no animation", "Manual placement, static scenes"],
      ["Bounce", "Azimuth and elevation oscillate back and forth", "Vertical bouncing motion"],
      ["Circle", "Horizontal orbit at constant elevation and distance", "Smooth circular panning"],
      ["Cross", "Alternates between azimuth and elevation axes", "X-pattern motion"],
      ["Figure-8", "Two-tangent-circles pattern in azimuth/elevation", "Elegant weaving motion"],
      ["Heart", "Cardioid-shaped path around the origin point", "Expressive, organic arcs"],
      ["Helix", "Spiral with rising/falling elevation over time", "3D corkscrew motion"],
      ["Infinity", "Horizontal figure-8 (lemniscate) path", "Smooth side-to-side weave"],
      ["Line", "Back-and-forth along a single azimuth axis", "Pendulum-like motion"],
      ["Orbit", "Circular path around the listener at constant height", "Rotating delays, circular ping-pong"],
      ["Random", "Multi-sine noise on all axes (never repeats)", "Organic, unpredictable movement"],
      ["Spiral", "Outward spiral with distance variation", "Expanding/contracting patterns"],
      ["Square", "Rectangular path rotated by base azimuth", "Angular, geometric motion"],
      ["Triangle", "Triangular path rotated by base azimuth", "Sharp directional changes"],
    ],
    { colWidths: [1500, 4000, 3526] }
  ));
  items.push(spacer(8));

  items.push(bodyPara([
    boldText("Speed "),
    bodyText("controls the animation rate (0.00\u20135.00 Hz). "),
    boldText("Direction "),
    bodyText("can be Forward or Reverse. Each tap animates independently \u2014 mix different shapes and speeds for complex spatial motion."),
  ]));

  items.push(bodyPara([
    bodyText("If "),
    internalLink("ADM-OSC receive", "adm-osc"),
    bodyText(" is active and an external controller sends position data for a tap, the trajectory pauses for that tap. It automatically resumes 500ms after the last OSC message."),
  ]));

  items.push(spacer(6));
  items.push(calloutBox("Creative tip", [
    "Try setting 4 taps on Orbit at different speeds (0.5\u00d7, 1.0\u00d7, 1.5\u00d7, 2.0\u00d7) for a mesmerizing rotating delay pattern. With feedback above 50%, the trails create evolving spatial textures.",
  ], C.amber));

  return items;
}

// ============================================================================
// SECTION 7: CREATIVE TIPS
// ============================================================================

function buildCreativeTips() {
  const items = [];
  items.push(heading1("Creative Tips", "creative-tips"));

  items.push(bodyPara("OpenSpatialDelay can go far beyond basic delay effects. Here are some techniques to explore:"));

  items.push(spacer(6));

  items.push(calloutBox("Self-Oscillation", [
    "Set Feedback above 95% to create self-oscillating delay trails that sustain indefinitely. The built-in output limiter (soft saturation at +2 dB) prevents digital clipping.",
    "For analog-style oscillation, enable the TONE filter and set LP to 2\u20133 kHz. The delay will sing with a warm, filtered tone. Adjust FEEDBACK to control the balance between sustain and decay.",
  ], C.amber));
  items.push(spacer(6));

  items.push(calloutBox("Pitch Spiraling (Shimmer)", [
    "Set global PITCH to +12 semitones (+1 octave). Each tap accumulates pitch: tap 1 = +1 octave, tap 2 = +2 octaves, and so on. Each feedback cycle adds another round-trip of pitch.",
    "For subtler shimmer, try +5 or +7 semitones (a fourth or fifth). Combined with high feedback and LP filtering, this creates lush, cathedral-like textures.",
  ], C.violet));
  items.push(spacer(6));

  items.push(calloutBox("Wobble \u2014 Tape Effects", [
    "Enable MOD and set AMOUNT to 15\u201325% with MORPH at 0% (sine wave). This creates a subtle tape-like wow and flutter on the delay.",
    "Higher MORPH values (toward 100%) produce more rhythmic, square-wave modulation \u2014 useful for choppy, glitchy delay effects.",
  ], C.rose));
  items.push(spacer(6));

  items.push(calloutBox("Stereo Width with Input Routing", [
    "On stereo tracks, try setting alternating taps to L and R input channels: Tap 1 = L, Tap 2 = R, Tap 3 = L, and so on.",
    "This creates a natural stereo ping-pong enhanced by spatial positioning. Position L-channel taps on the left side of the spatial map and R-channel taps on the right for maximum width.",
  ], C.green));
  items.push(spacer(6));

  items.push(calloutBox("Feedback Filter Design", [
    "The TONE section filters apply to the feedback path \u2014 each time the signal recirculates, it passes through the filters again. Over successive echoes, this creates progressive tonal shaping.",
    "High-pass at 200\u2013400 Hz removes bass buildup in long feedback tails. Low-pass at 3\u20135 kHz simulates tape machine degradation. Combine both for a focused, telephonic character that opens up beautifully in 3D space.",
  ], C.cyan));

  return items;
}

// ============================================================================
// SECTION 8: PRESETS
// ============================================================================

function buildPresets() {
  const items = [];
  items.push(heading1("Presets", "presets"));

  items.push(bodyPara([
    bodyText("OpenSpatialDelay includes "),
    boldText("70 factory presets"),
    bodyText(" across 9 categories: Classic Delays, Spatial Movement, Ambient + Texture, Height + 3D, Surround Production, Wobble + Modulated, Creative + Experimental, Rhythmic, and User."),
  ]));

  items.push(bodyPara([
    bodyText("Browse presets using the dropdown in the header bar. Use the "),
    boldText("< > arrows"),
    bodyText(" to step through presets, or click the preset name to open the full categorized menu."),
  ]));

  items.push(heading2("Preset File Locations", "preset-locations"));
  items.push(bodyPara([
    boldText("macOS: "),
    monoText("~/Library/Audio/Presets/OpenSpatialDelay/"),
  ]));
  items.push(bodyPara([
    boldText("Windows: "),
    monoText("%APPDATA%\\OpenSpatialDelay\\Presets\\"),
  ]));
  items.push(bodyPara("Factory presets are written to this directory on first launch. User presets are saved as JSON files in the same location."));

  items.push(heading2("Saving Presets", "saving-presets"));
  items.push(bodyPara([
    bodyText("Click the "),
    boldText("Save"),
    bodyText(" button in the header to open the save overlay. Enter a name and press Return to save, or Escape to cancel."),
  ]));

  return items;
}

// ============================================================================
// SECTION 9: ADM-OSC
// ============================================================================

function buildADMOSC() {
  const items = [];
  items.push(heading1("ADM-OSC Integration", "adm-osc"));

  items.push(bodyPara([
    bodyText("OpenSpatialDelay supports "),
    boldText("ADM-OSC"),
    bodyText(" (Audio Definition Model over Open Sound Control), the industry-standard protocol for controlling spatial audio object positions in real time."),
  ]));

  items.push(heading2("Receive (Incoming Control)"));
  items.push(bodyPara([
    bodyText("Enable "),
    boldText("RECV"),
    bodyText(" in the OSC section and set the port (default 4002). External tools like SPAT Revolution, L-ISA Controller, or custom scripts can send position data to control tap positions in real time. Supported messages include "),
    monoText("/adm/obj/N/azim"),
    bodyText(", "),
    monoText("/adm/obj/N/elev"),
    bodyText(", "),
    monoText("/adm/obj/N/dist"),
    bodyText(", "),
    monoText("/adm/obj/N/aed"),
    bodyText(" (combined), and "),
    monoText("/adm/obj/N/xyz"),
    bodyText(" (Cartesian, auto-converted to polar)."),
  ]));

  items.push(heading2("Send (Broadcast Positions)"));
  items.push(bodyPara([
    bodyText("Enable "),
    boldText("SEND"),
    bodyText(" and set the target IP and port (default 127.0.0.1:4003). The plugin broadcasts tap positions as "),
    monoText("/adm/obj/N/aed"),
    bodyText(" messages at 30 Hz, with position-change gating to minimize network traffic. Use this to sync with external spatial renderers."),
  ]));

  return items;
}

// ============================================================================
// SECTION 10: TROUBLESHOOTING
// ============================================================================

function buildTroubleshooting() {
  const items = [];
  items.push(heading1("Troubleshooting", "troubleshooting"));

  items.push(dataTable(
    ["Issue", "Cause", "Solution"],
    [
      ["No sound output", "All taps disabled", "Enable at least one tap (click ON button in bottom panel)"],
      ["Only dry sound, no echoes", "Dry/Wet at 0% or format mismatch", "Set Dry/Wet above 0%. Ensure track channel count matches selected Output Format."],
      ["Binaural doesn't sound 3D", "Wrong HRTF profile for your hearing", "Try different HRTF profiles. Start with Studio Reference."],
      ["Audio crackling/dropouts", "Buffer size too small or too many HRTF taps", "Increase DAW buffer to 256+ samples. Switch to Simple profile for lower CPU."],
      ["Plugin not appearing in DAW", "Installation path issue", "Verify plugins are in ~/Library/Audio/Plug-Ins/ (macOS). Rescan in DAW."],
      ["Delay not syncing to tempo", "Tempo Sync is off", "Enable the SYNC toggle in the DELAY section"],
      ["Runaway self-oscillation", "Very high feedback + resonant filters", "The output limiter prevents clipping, but reduce Feedback or filter resonance if too intense."],
      ["OSC not connecting", "Port conflict or wrong number", "Check that no other app uses the same port. Verify port matches sender/receiver."],
    ],
    { colWidths: [2200, 2600, 4226] }
  ));

  return items;
}

// ============================================================================
// SECTION 11: GLOSSARY
// ============================================================================

function glossaryEntry(term, definition) {
  return new Paragraph({
    spacing: { after: 120, line: 280 },
    indent: { left: 200, hanging: 200 },
    children: [
      new TextRun({ text: term, font: FONT_BODY, size: 20, color: C.navy, bold: true }),
      new TextRun({ text: ` \u2014 ${definition}`, font: FONT_BODY, size: 20, color: C.bodyText }),
    ],
  });
}

function buildGlossary() {
  const items = [];
  items.push(heading1("Glossary", "glossary"));
  items.push(bodyPara("Technical terms used throughout this manual, listed alphabetically."));
  items.push(spacer(4));

  const entries = [
    ["ACN (Ambisonic Channel Number)", "Standard channel ordering convention for Ambisonics. Used together with SN3D normalization, this combination is known as AmbiX."],
    ["ADM-OSC", "Audio Definition Model over Open Sound Control. An industry-standard protocol for controlling spatial audio object positions in real time over a network."],
    ["Air Absorption", "A DSP effect that simulates how high frequencies are naturally attenuated over distance in air. Farther sources sound progressively darker."],
    ["Ambisonics", "A full-sphere surround format that encodes 3D audio as spherical harmonics. Unlike channel-based formats, Ambisonics is speaker-layout-independent and can be decoded to any arrangement."],
    ["AmbiX", "The most common Ambisonics format, using ACN channel ordering with SN3D normalization. OpenSpatialDelay outputs in AmbiX format."],
    ["Attenuation", "A reduction in signal level (volume). Distance attenuation means sounds get quieter as they move farther from the listener."],
    ["AU (Audio Unit)", "Apple's native plugin format for macOS and iOS. Used in Logic Pro, GarageBand, and other Apple-ecosystem DAWs."],
    ["Azimuth", "Horizontal angle around the listener. 0\u00b0 = directly in front, +90\u00b0 = left, \u221290\u00b0 = right, \u00b1180\u00b0 = behind."],
    ["Binaural", "Audio rendered specifically for headphone listening. Uses HRTF processing to simulate 3D positioning by delivering different signals to each ear."],
    ["Cardioid", "A microphone polar pattern that picks up sound primarily from the front, rejecting sound from the rear. Heart-shaped sensitivity pattern."],
    ["Convolution", "A mathematical operation that combines two signals. In audio, it applies the characteristics of one signal (e.g., an HRTF impulse response) to another (your audio), producing realistic spatial or reverberant effects."],
    ["Cutoff Frequency", "The frequency at which a filter begins to attenuate. A low-pass filter with a 5 kHz cutoff passes frequencies below 5 kHz and progressively reduces those above."],
    ["DAW", "Digital Audio Workstation. Software for recording, editing, and producing audio (e.g., Reaper, Ableton Live, Logic Pro, Pro Tools)."],
    ["DBAP (Distance-Based Amplitude Panning)", "A panning algorithm that distributes audio to speakers based on each speaker's distance from the virtual source. Works with irregular speaker layouts."],
    ["Dolby Atmos", "An object-based spatial audio format by Dolby that supports height speakers. Common formats include 7.1.4 (7 ear-level + 1 LFE + 4 overhead speakers)."],
    ["Doppler Effect", "The pitch shift heard when a sound source moves relative to the listener. Approaching sources sound higher in pitch; receding sources sound lower. Think of a passing ambulance siren."],
    ["Elevation", "Vertical angle relative to the listener. 0\u00b0 = ear level (horizon), +90\u00b0 = directly above, \u221290\u00b0 = directly below."],
    ["Figure-8", "A microphone polar pattern that picks up sound equally from front and rear, rejecting sound from the sides. Also called bidirectional."],
    ["FOA (First-Order Ambisonics)", "The simplest Ambisonics format, using 4 channels (W, X, Y, Z). Provides basic 3D spatial encoding."],
    ["Gain", "The amount of amplification or attenuation applied to an audio signal, measured in decibels (dB). Positive gain = louder, negative gain = quieter."],
    ["High-Pass Filter (HP)", "A filter that allows frequencies above the cutoff to pass while attenuating lower frequencies. Used to remove low-end rumble or bass buildup."],
    ["HOA (Higher-Order Ambisonics)", "Ambisonics beyond first order (2nd, 3rd, ... 6th order). Higher orders use more channels but provide sharper spatial resolution and more precise source localization."],
    ["HRTF (Head-Related Transfer Function)", "Measurements of how sound is modified by the shape of your head, ears (pinnae), and torso as it travels from a source to your eardrums. Applied via convolution to simulate 3D audio over headphones."],
    ["ILD (Interaural Level Difference)", "The difference in volume between the two ears for a given sound source. Sources to the right are louder in the right ear. One of the two primary binaural cues (with ITD)."],
    ["ITD (Interaural Time Difference)", "The difference in arrival time between the two ears for a given sound source. Sound from the right reaches the right ear a fraction of a millisecond earlier. One of the two primary binaural cues (with ILD)."],
    ["KNN (K-Nearest Neighbors)", "A panning algorithm that routes audio to the K closest speakers to the virtual source position, weighted by distance. Useful for dense speaker arrays."],
    ["Latency", "The time delay between an audio input and its corresponding output. Lower latency feels more responsive. Measured in samples or milliseconds."],
    ["LFE (Low-Frequency Effects)", "A dedicated subwoofer channel in surround formats (the '.1' in 5.1). Carries only low-frequency content, typically below 120 Hz."],
    ["LFO (Low-Frequency Oscillator)", "An oscillator running below audible frequencies (typically 0.01\u201320 Hz) used to modulate parameters over time. In OpenSpatialDelay, the MOD section uses an LFO to modulate delay time, creating wobble effects."],
    ["Lissajous", "A mathematical curve formed by combining two oscillations at right angles. The Figure-8 trajectory in OpenSpatialDelay traces a Lissajous pattern with a 2:1 frequency ratio."],
    ["Low-Pass Filter (LP)", "A filter that allows frequencies below the cutoff to pass while attenuating higher frequencies. Used to darken or warm the sound."],
    ["MDAP (Multiple-Direction Amplitude Panning)", "An extension of VBAP that spreads the source across multiple directions using sub-sources, creating a wider, more diffuse image."],
    ["Object-Based Audio", "A spatial audio paradigm where each sound is an independent object with a 3D position, rather than being pre-mixed to fixed channels. ADM-OSC controls object positions in real time."],
    ["OSC (Open Sound Control)", "A network protocol for real-time communication between audio software and hardware. Used in OpenSpatialDelay for ADM-OSC position control."],
    ["Pan Law", "The gain curve applied when panning audio between speakers. A cosine/sine pan law maintains constant perceived loudness as a source moves across the stereo field."],
    ["Panning", "Distributing an audio signal across speakers (or virtual speakers) to create the perception of spatial position."],
    ["Pinna", "The visible outer part of the ear. Its complex folds create frequency-dependent filtering that the brain uses to determine sound direction, especially elevation. HRTF measurements capture these effects."],
    ["Pitch Shift", "Changing the perceived pitch of audio without changing its playback speed (or vice versa). Measured in semitones (st). +12 st = one octave up."],
    ["Resonance (Q)", "How much a filter emphasizes frequencies near its cutoff point. Higher resonance creates a sharper, more pronounced peak. At very high values, the filter can self-oscillate."],
    ["Self-Oscillation", "When a delay's feedback is set high enough that the signal sustains or grows indefinitely, creating continuous tones. OpenSpatialDelay includes a limiter to prevent clipping during self-oscillation."],
    ["Shimmer", "A delay/reverb effect where pitch is shifted upward on each feedback cycle, creating ethereal, rising textures. Achieved in OpenSpatialDelay by setting cumulative positive pitch shift with high feedback."],
    ["SN3D (Schmidt Semi-Normalization)", "A normalization scheme for spherical harmonics used in the AmbiX Ambisonics format. Ensures consistent signal levels across Ambisonics channels."],
    ["SOFA (Spatially Oriented Format for Acoustics)", "An open file format (.sofa) for storing spatially-oriented acoustic data, most commonly HRTF measurements. Each SOFA file contains hundreds of impulse responses measured at different directions around a listener's head."],
    ["Spherical Harmonics", "Mathematical functions defined on the surface of a sphere. Ambisonics uses spherical harmonics to encode the spatial distribution of sound, with higher orders providing finer spatial detail."],
    ["Surround Format Numbering", "The numbers in surround formats (e.g., 7.1.4) indicate: first number = ear-level speakers, second number = LFE subwoofers (0 or 1), third number = height/overhead speakers. So 7.1.4 = 7 ear-level + 1 sub + 4 overhead."],
    ["Tap", "In OpenSpatialDelay, a tap is a read point on the delay line. Each tap has its own 3D position, pitch shift, and other parameters. Up to 12 taps can be active simultaneously."],
    ["Tempo Sync", "Locking the delay time to the host DAW's tempo so that echoes fall on musical subdivisions (quarter notes, eighth notes, etc.)."],
    ["VBAP (Vector Base Amplitude Panning)", "A widely-used panning algorithm that places virtual sources using the nearest triangle of speakers. Produces focused, point-source images. Industry standard for object-based surround production."],
    ["VBIP (Vector Base Intensity Panning)", "A variant of VBAP that uses intensity-based gain computation instead of amplitude. Produces a smoother, broader spatial image compared to standard VBAP."],
    ["VST3 (Virtual Studio Technology 3)", "Steinberg's cross-platform plugin format, supported by most DAWs on macOS and Windows."],
    ["Woodworth Model", "A simplified mathematical model of the human head as a rigid sphere, used to calculate ITD and ILD for binaural audio. Used by OpenSpatialDelay's Simple (Low CPU) profile."],
  ];

  for (const [term, def] of entries) {
    items.push(glossaryEntry(term, def));
  }

  return items;
}

// ============================================================================
// SECTION 12: THIRD-PARTY NOTICES
// ============================================================================

function buildLegalNotices() {
  const items = [];
  items.push(heading1("Third-Party Notices", "legal-notices"));

  items.push(bodyPara("OpenSpatialDelay incorporates the following third-party software, data, and fonts. This section provides the required license notices and attributions."));

  // Software Libraries
  items.push(heading2("Software Libraries"));
  items.push(dataTable(
    ["Component", "License", "Copyright"],
    [
      ["JUCE 8 (Audio Plugin Framework)", "GPL-3.0 / Commercial", "Raw Material Software Limited"],
      ["libmysofa v1.3.2 (SOFA File Reader)", "BSD-3-Clause", "Christian Hoene, Symonics GmbH"],
      ["zlib (Compression)", "zlib License", "Jean-loup Gailly, Mark Adler"],
    ],
    { colWidths: [3600, 2400, 3026] }
  ));
  items.push(spacer(8));

  items.push(bodyPara([
    boldText("JUCE: "),
    bodyText("JUCE is dual-licensed under the GNU General Public License v3.0 and a commercial license. OpenSpatialDelay uses JUCE under the GPL-3.0. See "),
    externalLink("juce.com", "https://juce.com/"),
    bodyText(" for details."),
  ]));

  items.push(bodyPara([
    boldText("libmysofa: "),
    bodyText("Copyright (c) 2016-2024 Christian Hoene, Symonics GmbH. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: (1) Redistributions of source code must retain the above copyright notice, this list of conditions, and the following disclaimer. (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions, and the following disclaimer in the documentation and/or other materials provided with the distribution. (3) Neither the name of Symonics GmbH nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission."),
  ]));

  items.push(bodyPara([
    boldText("zlib: "),
    bodyText("Copyright (c) 1995-2024 Jean-loup Gailly and Mark Adler. This software is provided 'as-is', without any express or implied warranty. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions: (1) The origin of this software must not be misrepresented. (2) Altered source versions must be plainly marked as such. (3) This notice may not be removed or altered from any source distribution."),
  ]));

  // HRTF Datasets
  items.push(heading2("HRTF Datasets"));
  items.push(bodyPara("The following Head-Related Transfer Function datasets are embedded in the plugin for binaural spatialization:"));

  items.push(dataTable(
    ["Profile Name", "Dataset", "License", "Source"],
    [
      ["Studio Reference", "MIT KEMAR Large Pinna", "MIT", "MIT Media Lab"],
      ["Immersive", "SADIE II D2 KU100", "Apache 2.0", "University of York"],
      ["Natural", "CIPIC Subject 003", "Public Domain", "UC Davis CIPIC Lab"],
      ["Precise", "HUTUBS PP2", "CC BY 4.0", "TU Berlin"],
      ["Spatial", "Bernschuetz KU100 2\u00b0", "CC BY 3.0", "TH K\u00f6ln"],
    ],
    { colWidths: [1800, 2400, 1600, 3226] }
  ));
  items.push(spacer(8));

  items.push(bodyPara([
    boldText("MIT KEMAR (MIT License): "),
    bodyText("Measurements by the MIT Media Lab. Permission is hereby granted, free of charge, to any person obtaining a copy of this data, to deal in the data without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies, subject to the above copyright notice and this permission notice being included in all copies. See "),
    externalLink("sound.media.mit.edu/resources/KEMAR.html", "https://sound.media.mit.edu/resources/KEMAR.html"),
    bodyText("."),
  ]));

  items.push(bodyPara([
    boldText("SADIE II (Apache 2.0): "),
    bodyText("Gavin Kearney, Tony Doyle, University of York. Licensed under the Apache License, Version 2.0. You may obtain a copy of the License at "),
    externalLink("apache.org/licenses/LICENSE-2.0", "https://www.apache.org/licenses/LICENSE-2.0"),
    bodyText(". See "),
    externalLink("york.ac.uk/sadie-project/database2", "https://www.york.ac.uk/sadie-project/database2.html"),
    bodyText("."),
  ]));

  items.push(bodyPara([
    boldText("CIPIC Subject 003 (Public Domain): "),
    bodyText("UC Davis CIPIC Interface Laboratory. This dataset is in the public domain with no restrictions on use. See "),
    externalLink("ucdavis.edu/cipic", "https://www.ece.ucdavis.edu/cipic/"),
    bodyText("."),
  ]));

  items.push(bodyPara([
    boldText("HUTUBS PP2 (CC BY 4.0): "),
    bodyText("Fabian Brinkmann, Alexander Lindau, Stefan Weinzierl, et al., TU Berlin Audio Communication Group. Licensed under the Creative Commons Attribution 4.0 International License. You must give appropriate credit, provide a link to the license, and indicate if changes were made. See "),
    externalLink("depositonce.tu-berlin.de", "https://depositonce.tu-berlin.de/items/21a596ec-b7af-4e48-a8ed-4c4d68ce57a5"),
    bodyText("."),
  ]));

  items.push(bodyPara([
    boldText("Bernschuetz KU100 (CC BY 3.0): "),
    bodyText("Benjamin Bernsch\u00fctz, TH K\u00f6ln (Cologne University of Applied Sciences). Licensed under the Creative Commons Attribution 3.0 Unported License. You must give appropriate credit and indicate if changes were made. See "),
    externalLink("audiogroup.web.th-koeln.de", "https://audiogroup.web.th-koeln.de/"),
    bodyText("."),
  ]));

  // Fonts
  items.push(heading2("Fonts"));
  items.push(dataTable(
    ["Font", "License", "Copyright"],
    [
      ["DM Sans (Regular, Medium, SemiBold, Bold)", "SIL Open Font License 1.1", "Colophon Foundry, Google"],
      ["JetBrains Mono (Regular, Medium, Bold)", "SIL Open Font License 1.1", "JetBrains s.r.o."],
      ["Roboto (Medium)", "Apache License 2.0", "Google LLC"],
    ],
    { colWidths: [3600, 2400, 3026] }
  ));
  items.push(spacer(8));

  items.push(bodyPara([
    boldText("SIL Open Font License 1.1: "),
    bodyText("DM Sans and JetBrains Mono are licensed under the SIL Open Font License, Version 1.1. This license is available at "),
    externalLink("openfontlicense.org", "https://openfontlicense.org/"),
    bodyText(". The fonts may be used, modified, and distributed freely, provided that they are not sold by themselves and any derivative fonts carry a different name."),
  ]));

  items.push(bodyPara([
    boldText("Apache License 2.0: "),
    bodyText("Roboto is licensed under the Apache License, Version 2.0. You may obtain a copy of the License at "),
    externalLink("apache.org/licenses/LICENSE-2.0", "https://www.apache.org/licenses/LICENSE-2.0"),
    bodyText("."),
  ]));

  // Project License note
  items.push(heading2("Project License"));
  items.push(bodyPara([
    bodyText("OpenSpatialDelay is free software, licensed under the "),
    boldText("GNU General Public License v3.0 (GPL-3.0)"),
    bodyText(". The full GPL-3.0 text is included in the LICENSE file distributed with the source code."),
  ]));

  return items;
}

// ============================================================================
// SECTION 13: BACK PAGE
// ============================================================================

function buildBackPage() {
  const items = [];

  items.push(spacer(60));

  if (logoImg) {
    items.push(new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 300 },
      children: [new ImageRun({
        type: "png",
        data: logoImg,
        transformation: { width: 160, height: 81 },
        altText: { title: "SML Logo", description: "Spatial Media Lab Logo", name: "sml-logo-back" },
      })],
    }));
  }

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 100 },
    children: [new TextRun({ text: "OpenSpatialDelay v1.0", font: FONT_BODY, size: 32, color: C.navy, bold: true })],
  }));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 200 },
    children: [new TextRun({ text: "Part of the Spatial Media Library Suite", font: FONT_BODY, size: 22, color: C.dimText })],
  }));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 100 },
    children: [
      new TextRun({ text: "Designed by Andrew Rahman  \u00b7  Architecture by Claude", font: FONT_BODY, size: 20, color: C.subheading }),
    ],
  }));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 60 },
    children: [
      externalLink("spatialmedialab.org", "https://spatialmedialab.org"),
    ],
  }));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 60 },
    children: [
      externalLink("github.com/AndrewRahman/OpenSpatialDelay", "https://github.com/AndrewRahman/OpenSpatialDelay"),
    ],
  }));

  items.push(spacer(40));

  items.push(new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 60 },
    children: [new TextRun({ text: "Open Source  \u00b7  2026", font: FONT_BODY, size: 18, color: C.dimText })],
  }));

  return items;
}

// ============================================================================
// DOCUMENT ASSEMBLY
// ============================================================================

async function buildManual() {
  console.log("Building OpenSpatialDelay v1.0 User Manual...");

  // Numbering config for bullet lists
  const numbering = {
    config: [
      {
        reference: "bullets",
        levels: [{
          level: 0,
          format: LevelFormat.BULLET,
          text: "\u2022",
          alignment: AlignmentType.LEFT,
          style: { paragraph: { indent: { left: 720, hanging: 360 } } },
        }],
      },
    ],
  };

  // Styles
  const styles = {
    default: {
      document: {
        run: { font: FONT_BODY, size: 22, color: C.bodyText },
      },
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
      {
        id: "Hyperlink", name: "Hyperlink",
        run: { color: C.link, underline: {} },
      },
    ],
  };

  // Build TOC first, then reset numbering for content
  const tocChildren = buildTOC();
  sn.reset();

  // Build all content sections (sn auto-increments in same order as TOC)
  const mainContent = [
    ...buildQuickStart(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildWhatIs(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildSpatialMap(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildOutputFormats(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildControlsReference(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildTrajectories(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildCreativeTips(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildPresets(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildADMOSC(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildTroubleshooting(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildLegalNotices(),
    new Paragraph({ children: [new PageBreak()] }),
    ...buildBackPage(),
  ];

  // Build document with sections
  const doc = new Document({
    styles,
    numbering,
    sections: [
      // COVER PAGE (separate section, no header/footer)
      {
        properties: {
          page: {
            size: { width: A4_W, height: A4_H },
            margin: { top: MARGIN, right: MARGIN, bottom: MARGIN, left: MARGIN },
          },
        },
        children: buildCover(),
      },
      // TABLE OF CONTENTS
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
                new TextRun({ text: "OpenSpatialDelay v1.0", font: FONT_BODY, size: 16, color: C.dimText }),
              ],
              border: { bottom: { style: BorderStyle.SINGLE, size: 4, color: C.navy, space: 4 } },
            })],
          }),
        },
        footers: {
          default: new Footer({
            children: [new Paragraph({
              alignment: AlignmentType.CENTER,
              children: [
                new TextRun({ text: "Page ", font: FONT_BODY, size: 16, color: C.dimText }),
                new TextRun({ children: [PageNumber.CURRENT], font: FONT_BODY, size: 16, color: C.dimText }),
              ],
            })],
          }),
        },
        children: tocChildren,
      },
      // MAIN CONTENT (all remaining sections in one block)
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
                new TextRun({ text: "OpenSpatialDelay v1.0  \u2014  User Manual", font: FONT_BODY, size: 16, color: C.dimText }),
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

  // Generate buffer and write
  const buffer = await Packer.toBuffer(doc);
  fs.writeFileSync(OUTPUT_PATH, buffer);
  console.log(`Manual generated: ${OUTPUT_PATH}`);
  console.log(`File size: ${(buffer.length / 1024).toFixed(1)} KB`);
}

// Run
buildManual().catch(err => {
  console.error("Error generating manual:", err);
  process.exit(1);
});
