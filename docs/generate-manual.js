// OpenSpatialDelay User Manual Generator
// Generates a professional .docx manual using the docx npm package (v9.6.1)

const {
  Document,
  Packer,
  Paragraph,
  TextRun,
  HeadingLevel,
  AlignmentType,
  TableOfContents,
  PageBreak,
  Table,
  TableRow,
  TableCell,
  WidthType,
  ShadingType,
  BorderStyle,
  Header,
  Footer,
  PageNumber,
  NumberFormat,
  LevelFormat,
  convertInchesToTwip,
  Tab,
  TabStopPosition,
  TabStopType,
  VerticalAlign,
} = require("docx");
const fs = require("fs");

// ─── Color palette ──────────────────────────────────────────────
const COLORS = {
  darkNavy: "1a1a2e",
  darkGray: "333333",
  medGray: "666666",
  lightGray: "999999",
  tableHeaderBg: "1a1a2e",
  tableHeaderText: "FFFFFF",
  tableAltRow: "f0f4f8",
  tableWhiteRow: "FFFFFF",
  accent: "00a8cc",
  white: "FFFFFF",
  black: "000000",
};

// ─── Font helpers ───────────────────────────────────────────────
const FONT = "Arial";
const BODY_SIZE = 22; // 11pt in half-points
const SMALL_SIZE = 18; // 9pt
const H1_SIZE = 36; // 18pt
const H2_SIZE = 28; // 14pt
const H3_SIZE = 24; // 12pt

// ─── Reusable paragraph builders ────────────────────────────────

function bodyText(text, opts = {}) {
  return new Paragraph({
    spacing: { after: 120 },
    ...(opts.paragraphOpts || {}),
    children: [
      new TextRun({
        text,
        font: FONT,
        size: BODY_SIZE,
        color: COLORS.darkGray,
        ...(opts.runOpts || {}),
      }),
    ],
  });
}

function bodyParagraph(runs, opts = {}) {
  return new Paragraph({
    spacing: { after: 120 },
    ...opts,
    children: runs,
  });
}

function textRun(text, opts = {}) {
  return new TextRun({
    font: FONT,
    size: BODY_SIZE,
    color: COLORS.darkGray,
    ...opts,
  });
}

function boldRun(text, opts = {}) {
  return textRun(text, { bold: true, ...opts });
}

function heading1(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_1,
    spacing: { before: 360, after: 200 },
    children: [
      new TextRun({
        text,
        font: FONT,
        size: H1_SIZE,
        bold: true,
        color: COLORS.darkNavy,
      }),
    ],
  });
}

function heading2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2,
    spacing: { before: 280, after: 160 },
    children: [
      new TextRun({
        text,
        font: FONT,
        size: H2_SIZE,
        bold: true,
        color: COLORS.darkNavy,
      }),
    ],
  });
}

function heading3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3,
    spacing: { before: 200, after: 120 },
    children: [
      new TextRun({
        text,
        font: FONT,
        size: H3_SIZE,
        bold: true,
        color: COLORS.darkGray,
      }),
    ],
  });
}

function emptyPara() {
  return new Paragraph({ spacing: { after: 60 }, children: [] });
}

function pageBreak() {
  return new Paragraph({
    children: [new PageBreak()],
  });
}

// ─── Bullet list builder ────────────────────────────────────────
// We use numbering config for bullet lists

const BULLET_REF = "bullet-list";

function bulletItem(text, level = 0) {
  // text can be a string or array of TextRun
  const children =
    typeof text === "string"
      ? [textRun(text)]
      : text;
  return new Paragraph({
    numbering: { reference: BULLET_REF, level },
    spacing: { after: 60 },
    children,
  });
}

// ─── Table builder ──────────────────────────────────────────────

function makeTable(headers, rows, colWidths) {
  const totalWidth = colWidths.reduce((a, b) => a + b, 0);

  function headerCell(text, width) {
    return new TableCell({
      width: { size: width, type: WidthType.DXA },
      shading: {
        type: ShadingType.CLEAR,
        fill: COLORS.tableHeaderBg,
        color: COLORS.tableHeaderBg,
      },
      margins: {
        top: 60,
        bottom: 60,
        left: 80,
        right: 80,
      },
      verticalAlign: VerticalAlign.CENTER,
      children: [
        new Paragraph({
          spacing: { after: 0 },
          children: [
            new TextRun({
              text,
              font: FONT,
              size: 20,
              bold: true,
              color: COLORS.tableHeaderText,
            }),
          ],
        }),
      ],
    });
  }

  function dataCell(text, width, isAlt) {
    // text can be string or array of TextRun
    const children =
      typeof text === "string"
        ? [
            new TextRun({
              text,
              font: FONT,
              size: 20,
              color: COLORS.darkGray,
            }),
          ]
        : text;
    return new TableCell({
      width: { size: width, type: WidthType.DXA },
      shading: {
        type: ShadingType.CLEAR,
        fill: isAlt ? COLORS.tableAltRow : COLORS.tableWhiteRow,
        color: isAlt ? COLORS.tableAltRow : COLORS.tableWhiteRow,
      },
      margins: {
        top: 50,
        bottom: 50,
        left: 80,
        right: 80,
      },
      verticalAlign: VerticalAlign.CENTER,
      children: [
        new Paragraph({
          spacing: { after: 0 },
          children,
        }),
      ],
    });
  }

  const headerRow = new TableRow({
    tableHeader: true,
    children: headers.map((h, i) => headerCell(h, colWidths[i])),
  });

  const dataRows = rows.map(
    (row, ri) =>
      new TableRow({
        children: row.map((cell, ci) =>
          dataCell(cell, colWidths[ci], ri % 2 === 1)
        ),
      })
  );

  return new Table({
    width: { size: totalWidth, type: WidthType.DXA },
    columnWidths: colWidths,
    rows: [headerRow, ...dataRows],
  });
}

// ─── Divider paragraph ─────────────────────────────────────────
function divider() {
  return new Paragraph({
    spacing: { before: 200, after: 200 },
    border: {
      bottom: {
        style: BorderStyle.SINGLE,
        size: 6,
        color: "cccccc",
      },
    },
    children: [],
  });
}

// ═══════════════════════════════════════════════════════════════
//  CHAPTER CONTENT
// ═══════════════════════════════════════════════════════════════

function coverPage() {
  return [
    emptyPara(),
    emptyPara(),
    emptyPara(),
    emptyPara(),
    emptyPara(),
    emptyPara(),
    new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 100 },
      children: [
        new TextRun({
          text: "OpenSpatialDelay",
          font: FONT,
          size: 56, // 28pt
          bold: true,
          color: COLORS.darkNavy,
        }),
      ],
    }),
    new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 80 },
      children: [
        new TextRun({
          text: "User Manual",
          font: FONT,
          size: 36, // 18pt
          color: COLORS.darkNavy,
        }),
      ],
    }),
    new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 80 },
      children: [
        new TextRun({
          text: "Version 0.6",
          font: FONT,
          size: 28, // 14pt
          color: COLORS.medGray,
        }),
      ],
    }),
    emptyPara(),
    new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 80 },
      children: [
        new TextRun({
          text: "A spatial audio delay plugin for immersive mixing",
          font: FONT,
          size: 24, // 12pt
          italics: true,
          color: COLORS.medGray,
        }),
      ],
    }),
    emptyPara(),
    emptyPara(),
    emptyPara(),
    new Paragraph({
      alignment: AlignmentType.CENTER,
      spacing: { after: 80 },
      children: [
        new TextRun({
          text: "Spatial Media Library",
          font: FONT,
          size: BODY_SIZE,
          color: COLORS.darkGray,
        }),
      ],
    }),
    pageBreak(),
  ];
}

function tocPage() {
  return [
    heading1("Table of Contents"),
    new TableOfContents("Table of Contents", {
      hyperlink: true,
      headingStyleRange: "1-3",
    }),
    pageBreak(),
  ];
}

// ─── Chapter 1: Introduction ────────────────────────────────────
function chapter1() {
  return [
    heading1("1. Introduction"),

    heading2("1.1 What Is OpenSpatialDelay?"),
    bodyText(
      "OpenSpatialDelay is a spatial audio delay plugin built with the JUCE framework. It places up to 12 independently positioned delay taps in 3D space, enabling sound designers, immersive music producers, post-production mixers, and sound artists to create spatially rich delay effects."
    ),
    bodyText(
      "Unlike traditional delay plugins that operate in mono or stereo, OpenSpatialDelay renders each delay tap as a discrete spatial object with its own azimuth, elevation, and distance. The result is a delay effect that exists in three-dimensional space, whether monitored on headphones via binaural rendering or through multichannel speaker systems."
    ),

    heading2("1.2 Who Is It For?"),
    bulletItem([
      boldRun("Spatial audio designers "),
      textRun("working on immersive installations and VR/AR experiences"),
    ]),
    bulletItem([
      boldRun("Immersive music producers "),
      textRun("creating spatial mixes in Dolby Atmos, Ambisonics, or binaural"),
    ]),
    bulletItem([
      boldRun("Post-production mixers "),
      textRun("adding spatial depth to film, broadcast, and game audio"),
    ]),
    bulletItem([
      boldRun("Sound artists "),
      textRun("exploring generative spatial textures and trajectory-based motion"),
    ]),

    heading2("1.3 Key Capabilities"),
    bulletItem("12 independently positionable delay taps in 3D space"),
    bulletItem("7 spatialization algorithms (VBAP, VBIP, DBAP, KNN, MDAP, Ambisonics HOA, Direct Binaural) plus 5 stereo modes"),
    bulletItem("6 HRTF profiles for binaural monitoring (including SOFA-based convolution)"),
    bulletItem("21 output formats spanning Binaural, Stereo, Surround (Quad through 9.1.6), and Ambisonics (1st through 6th order)"),
    bulletItem("6 trajectory shapes for automated spatial motion (Spiral, Orbit, Bounce, Figure-8, Random, None)"),
    bulletItem("ADM-OSC receive for real-time external position control"),
    bulletItem("Per-tap pitch shifting, doppler effect, and per-feedback-cycle filtering"),
    bulletItem("8 factory presets and user preset save/load"),
    pageBreak(),
  ];
}

// ─── Chapter 2: Installation ────────────────────────────────────
function chapter2() {
  return [
    heading1("2. Installation"),

    heading2("2.1 System Requirements"),
    makeTable(
      ["Platform", "Minimum OS", "Architecture", "Plugin Formats"],
      [
        ["macOS", "12.0 (Monterey)+", "Apple Silicon (arm64)", "AU, VST3"],
        ["Windows", "10+", "x64", "VST3"],
      ],
      [2000, 2400, 2400, 2560]
    ),
    emptyPara(),
    bodyText(
      "A VST3 or AU compatible DAW is required, such as Logic Pro, Reaper, Nuendo, Ableton Live, or Pro Tools."
    ),

    heading2("2.2 Plugin Formats"),
    makeTable(
      ["Format", "Platform", "File Extension"],
      [
        ["Audio Unit (AU)", "macOS only", ".component"],
        ["VST3", "macOS and Windows", ".vst3"],
      ],
      [3120, 3120, 3120]
    ),

    heading2("2.3 Installation Paths"),
    heading3("macOS Audio Unit"),
    bodyText("~/Library/Audio/Plug-Ins/Components/OpenSpatialDelay v0.6.component"),
    heading3("macOS VST3"),
    bodyText("~/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay v0.6.vst3"),
    heading3("Windows VST3"),
    bodyText("C:\\Program Files\\Common Files\\VST3\\OpenSpatialDelay v0.6.vst3"),
    emptyPara(),
    bodyText(
      "After placing the plugin file in the appropriate directory, trigger a plugin rescan in your DAW if it does not appear automatically."
    ),

    heading2("2.4 Building from Source"),
    bodyText("OpenSpatialDelay can be built from source using CMake 3.22 or later:"),
    bodyText("cmake -B build -DCMAKE_BUILD_TYPE=Release", {
      runOpts: { font: "Courier New", size: 20, color: COLORS.darkGray },
    }),
    bodyText("cmake --build build --config Release", {
      runOpts: { font: "Courier New", size: 20, color: COLORS.darkGray },
    }),

    heading2("2.5 First Launch Verification"),
    bodyText(
      "When loaded for the first time, the plugin initializes with the Default preset:"
    ),
    bulletItem("4 delay taps enabled in a diagonal cross pattern at -45, +45, -135, and +135 degrees azimuth"),
    bulletItem("Algorithm: VBAP (Vector Base Amplitude Panning)"),
    bulletItem("Output format: Binaural"),
    bulletItem("Delay time: 500 ms, Feedback: 30%, Dry/Wet: 50%"),
    bulletItem("All taps at 0 degrees elevation and 0.5 normalized distance"),
    emptyPara(),
    bodyText(
      "Play audio through the track to confirm the 4-tap spatial delay is audible. Drag objects on the spatial map to verify interactive positioning."
    ),
    pageBreak(),
  ];
}

// ─── Chapter 3: Interface Walkthrough ───────────────────────────
function chapter3() {
  return [
    heading1("3. Interface Walkthrough"),
    bodyText(
      "The OpenSpatialDelay interface is divided into four zones arranged around a central spatial map. The window size is 820 x 580 pixels."
    ),

    heading2("3.1 Window Layout"),
    bodyText(
      "The interface is organized into four zones: the Header Bar across the top (56px), the Spatial Map in the center, the Right Panel (264px wide) containing delay, tone, and mix controls, and the Bottom Panel (150px) for per-object selection and editing."
    ),

    heading2("3.2 Header Bar"),
    bodyText("The header bar spans the full width and contains these elements from left to right:"),
    makeTable(
      ["Element", "Description"],
      [
        ["Plugin Title", "\"OpenSpatialDelay v0.6\" displayed left-aligned"],
        [
          "Preset Browser",
          "ComboBox dropdown (130px) + Prev/Next buttons (24px each) + Save button (44px)",
        ],
        [
          "OSC Toggle + Port",
          "\"OSC\" toggle button (42px) + editable port label (50px, default 4002)",
        ],
        [
          "Output Format",
          "Dropdown (120px) selecting the output format (Binaural, Stereo, Surround, Ambisonics)",
        ],
        [
          "Algorithm / HRTF Profile",
          "Dropdown (120px) \u2014 shows Algorithm for surround output, or HRTF Profile for binaural. Only one is visible at a time.",
        ],
      ],
      [2600, 6760]
    ),

    heading2("3.3 Spatial Map"),
    bodyText(
      "The spatial map is a top-down polar view of the 3D sound field, occupying the center of the interface."
    ),
    bulletItem("Orientation labels: F (front/top), B (back/bottom), L (left), R (right)"),
    bulletItem("Distance rings: 4 concentric rings at 0.25, 0.50, 0.75, and 1.0 normalized distance"),
    bulletItem("Center dot: White dot at the origin representing the listener position"),
    bulletItem("Objects: Colored numbered dots that can be dragged to reposition in azimuth and distance"),
    bulletItem([
      boldRun("Elevation encoding (IEM standard): "),
      textRun("Objects above ear level appear as larger, brighter dots (full opacity). Objects below ear level appear as smaller, dimmer dots (0.3 alpha). Dot size varies by +/- 3 pixels based on elevation angle."),
    ]),
    bulletItem([
      boldRun("OSC override indicator: "),
      textRun("When an object is receiving ADM-OSC position data, an \"OSC\" label appears in cyan beneath the dot."),
    ]),

    heading2("3.4 Right Panel"),
    heading3("DELAY Section"),
    makeTable(
      ["Control", "Description"],
      [
        ["Input Gain", "-60 to +12 dB (default 0 dB)"],
        [
          "Delay Time",
          "1 to 2000 ms (default 500 ms). Replaced by Note Division knob when Tempo Sync is active.",
        ],
        [
          "Tempo Sync",
          "Toggle button. When ON, delay time locks to musical note divisions.",
        ],
        ["Sync Mode", "Dropdown: Notes, Triplet, Dotted, 16th"],
        ["Feedback", "0% to 100% (default 30%)"],
        ["Pitch Shift", "-24 to +24 semitones (default 0 st)"],
      ],
      [2600, 6760]
    ),
    emptyPara(),

    heading3("TONE Section"),
    makeTable(
      ["Control", "Description"],
      [
        ["HP Filter", "High-pass, 20 Hz to 5000 Hz (default 20 Hz)"],
        ["LP Filter", "Low-pass, 200 Hz to 20000 Hz (default 20000 Hz)"],
        ["AIR", "Air Absorption toggle (right-aligned in section header)"],
      ],
      [2600, 6760]
    ),
    emptyPara(),

    heading3("MIX Section"),
    makeTable(
      ["Control", "Description"],
      [
        ["Dry/Wet", "0% to 100% (default 50%)"],
        ["Output Gain", "-60 to +12 dB (default 0 dB)"],
      ],
      [2600, 6760]
    ),

    heading2("3.5 Bottom Panel"),
    heading3("Object Selector Row"),
    bodyText(
      "12 color-coded buttons are arranged horizontally. Each button selects the corresponding delay tap for editing. Enabled objects show their number in full color; disabled objects appear dimmed. Object colors are evenly distributed across the hue spectrum:"
    ),
    makeTable(
      ["Object", "Color", "Object", "Color"],
      [
        ["1", "Red", "7", "Cyan"],
        ["2", "Orange", "8", "Azure"],
        ["3", "Yellow", "9", "Blue"],
        ["4", "Chartreuse", "10", "Violet"],
        ["5", "Green", "11", "Magenta"],
        ["6", "Spring", "12", "Rose"],
      ],
      [1600, 2100, 1600, 2100]
    ),
    emptyPara(),

    heading3("Per-Object Controls"),
    bodyText("When an object is selected, the following controls appear:"),
    makeTable(
      ["Control", "Description"],
      [
        ["ON/OFF", "Enable/disable toggle for this delay tap"],
        ["Azimuth", "-180 to +180 degrees (rotary knob)"],
        ["Elevation", "-90 to +90 degrees (vertical slider)"],
        ["Distance", "0.0 to 1.0 normalized (rotary knob)"],
        ["Doppler", "0% to 100% doppler effect amount (rotary knob)"],
        ["Trajectory", "Shape dropdown (None, Spiral, Orbit, Bounce, Figure-8, Random)"],
        ["Speed", "Trajectory speed 0.0 to 10.0 (rotary knob)"],
      ],
      [2600, 6760]
    ),
    pageBreak(),
  ];
}

// ─── Chapter 4: Delay Engine ────────────────────────────────────
function chapter4() {
  return [
    heading1("4. Delay Engine"),

    heading2("4.1 Signal Flow"),
    bodyText(
      "The delay engine processes audio through the following chain:"
    ),
    bodyText("Input \u2192 Input Gain \u2192 Delay Line \u2192 Feedback Loop \u2192 Spatial Rendering \u2192 Dry/Wet Mix \u2192 Output Gain \u2192 Output", {
      runOpts: { bold: true },
    }),
    emptyPara(),
    bodyText(
      "Each of the 12 delay taps maintains its own delay line. The feedback path includes high-pass and low-pass filtering plus optional pitch shifting, applied on each feedback cycle."
    ),

    heading2("4.2 Delay Time"),
    bulletItem("Range: 1 ms to 2000 ms (default 500 ms)"),
    bulletItem("The control uses a logarithmic skew (0.3) for finer control at shorter delay times"),
    bulletItem("Values at or above 1000 ms display in seconds"),
    bulletItem([
      boldRun("Per-tap delay times: "),
      textRun("Each object has its own delay time parameter. By default, Object 1 = 100 ms, Object 2 = 200 ms, and so on up to Object 12 = 1200 ms."),
    ]),

    heading2("4.3 Tempo Sync"),
    bodyText(
      "When Tempo Sync is enabled, the delay time locks to musical note divisions derived from the DAW's current tempo."
    ),
    makeTable(
      ["Division Value", "Note Length", "Sync Mode Options"],
      [
        ["1", "1/16 note", "Notes, Triplet, Dotted, 16th"],
        ["2", "1/8 note", "Notes, Triplet, Dotted, 16th"],
        ["4", "1/4 note", "Notes, Triplet, Dotted, 16th"],
        ["8", "1/2 note", "Notes, Triplet, Dotted, 16th"],
        ["16", "1/1 (whole note)", "Notes, Triplet, Dotted, 16th"],
      ],
      [2400, 3280, 3680]
    ),

    heading2("4.4 Feedback"),
    bodyText(
      "Feedback controls how much of the delayed signal is fed back into the delay line. Range: 0% to 100% (default 30%). Higher values produce more repetitions. At 100%, the delay sustains indefinitely (no signal decay)."
    ),
    bodyText(
      "The feedback path includes a high-pass filter (20 Hz to 5000 Hz, default 20 Hz) and a low-pass filter (200 Hz to 20000 Hz, default 20000 Hz). These filters shape the tonal character of successive delay repetitions, allowing for effects like progressive high-frequency rolloff."
    ),

    heading2("4.5 Pitch Shifting"),
    bodyText(
      "Pitch Shift applies a grain-based pitch shift on each feedback cycle. Range: -24 to +24 semitones (default 0 st). The effect is cumulative: each feedback repetition is shifted by the specified amount relative to the previous repetition. For example, +2 semitones means the first repeat is shifted +2 st, the second +4 st, and so on."
    ),
    pageBreak(),
  ];
}

// ─── Chapter 5: Spatial Positioning ─────────────────────────────
function chapter5() {
  return [
    heading1("5. Spatial Positioning"),

    heading2("5.1 Position Parameters"),
    makeTable(
      ["Parameter", "Range", "Default", "Description"],
      [
        ["Azimuth", "-180 to +180 degrees", "Varies per tap", "Horizontal angle. 0 = front, positive = left (IEM convention)."],
        ["Elevation", "-90 to +90 degrees", "0 degrees", "Vertical angle. Positive = above ear level."],
        ["Distance", "0.0 to 1.0", "0.5", "Normalized distance from center. 0 = center (listener), 1 = outer ring."],
      ],
      [1800, 2200, 1800, 3560]
    ),

    heading2("5.2 Spatial Map Interaction"),
    bodyText(
      "The spatial map provides a top-down polar view of all 12 delay taps. Interaction is straightforward:"
    ),
    bulletItem("Click on an object dot to select it for editing"),
    bulletItem("Drag an object dot to reposition it in azimuth (angular position) and distance (radial position)"),
    bulletItem("The selected object is highlighted and its per-object controls appear in the bottom panel"),
    bulletItem("Elevation cannot be adjusted via the map (it is a 2D projection); use the elevation slider in the bottom panel"),

    heading2("5.3 Elevation Encoding"),
    bodyText(
      "Since the spatial map is a 2D top-down view, elevation is encoded visually following the IEM standard:"
    ),
    bulletItem([
      boldRun("Above ear level: "),
      textRun("Objects appear as larger, brighter dots with full opacity fill"),
    ]),
    bulletItem([
      boldRun("Below ear level: "),
      textRun("Objects appear as smaller, dimmer dots with 0.3 alpha fill"),
    ]),
    bulletItem("Dot size varies by +/- 3 pixels based on elevation angle"),
    bulletItem("When an object is selected and has non-zero elevation (greater than +/-1 degree), the elevation in degrees is shown as a text label near the dot"),

    heading2("5.4 Coordinate Convention"),
    bodyText(
      "OpenSpatialDelay follows the convention where 0 degrees azimuth is directly in front of the listener, positive azimuth values move to the left, and negative values move to the right. This is consistent with the IEM plugin suite and common spatial audio toolkits."
    ),
    pageBreak(),
  ];
}

// ─── Chapter 6: Spatialization Algorithms ───────────────────────
function chapter6() {
  return [
    heading1("6. Spatialization Algorithms"),
    bodyText(
      "OpenSpatialDelay provides 7 core spatialization algorithms and 5 stereo panning modes. The active algorithm depends on the selected output format."
    ),

    heading2("6.1 Surround Algorithms"),
    bodyText("These 6 algorithms are available when the output format is a surround layout (Quad through 9.1.6):"),
    makeTable(
      ["Algorithm", "Technique", "Best For"],
      [
        [
          "Ambisonics (HOA)",
          "Spherical harmonic encoding (ACN/SN3D) with max-rE weighting",
          "Ambisonics bus output, flexible decoding, rotation-friendly workflows",
        ],
        [
          "DBAP",
          "Distance-Based Amplitude Panning (inverse-distance-squared)",
          "Irregular or non-standard speaker arrangements",
        ],
        [
          "KNN",
          "K-Nearest Neighbor panning with inverse-distance weighting",
          "General-purpose use with smooth spatial transitions",
        ],
        [
          "MDAP",
          "Multiple-Direction Amplitude Panning (8 spread sub-sources via VBAP)",
          "Wider, more stable spatial images; diffuse source rendering",
        ],
        [
          "VBAP",
          "Vector Base Amplitude Panning (2D pairs or 3D triplets)",
          "Precise localization with standard speaker layouts",
        ],
        [
          "VBIP",
          "Vector Base Intensity Panning (squared VBAP gains)",
          "Tighter source focus, reduced phantom image width",
        ],
      ],
      [2000, 4180, 3180]
    ),

    heading2("6.2 Stereo Modes"),
    bodyText("When the output format is Stereo, the algorithm parameter switches to stereo panning modes:"),
    makeTable(
      ["Mode", "Description"],
      [
        ["Equal Power", "Standard equal-power panning law"],
        ["Stereo VBAP", "VBAP applied to a virtual L/R speaker pair"],
        ["XY Pair", "Coincident XY microphone pair simulation"],
        ["MS Encode", "Mid-Side encoding (Mid = center, Side = azimuth offset)"],
        ["Blumlein", "Blumlein figure-8 pair simulation"],
      ],
      [2600, 6760]
    ),

    heading2("6.3 Algorithm Availability by Output Format"),
    makeTable(
      ["Output Format", "Available Algorithms", "Notes"],
      [
        [
          "Binaural",
          "Locked internally",
          "Uses Direct Binaural (Woodworth) for Simple profile, or per-source HRTF convolution for SOFA profiles. Algorithm dropdown is replaced by HRTF Profile selector.",
        ],
        [
          "Stereo",
          "Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein",
          "Algorithm dropdown shows stereo modes only.",
        ],
        [
          "Surround (Quad\u20139.1.6)",
          "Ambisonics HOA, DBAP, KNN, MDAP, VBAP, VBIP",
          "All 6 surround algorithms available.",
        ],
        [
          "Ambisonics (FOA\u20136OA)",
          "Locked to Ambisonics Encode",
          "Spherical harmonic encoding at the selected order. Algorithm dropdown not applicable.",
        ],
      ],
      [2400, 3280, 3680]
    ),

    heading2("6.4 Choosing an Algorithm"),
    bulletItem([
      boldRun("VBAP "),
      textRun("is the default and recommended starting point for most surround workflows. It provides precise localization with minimal CPU overhead."),
    ]),
    bulletItem([
      boldRun("DBAP "),
      textRun("is best when working with non-standard speaker arrangements where triangulation may fail."),
    ]),
    bulletItem([
      boldRun("KNN "),
      textRun("provides the smoothest transitions and is a good general-purpose choice."),
    ]),
    bulletItem([
      boldRun("MDAP "),
      textRun("trades localization precision for source width and stability, useful for diffuse or ambient effects."),
    ]),
    bulletItem([
      boldRun("VBIP "),
      textRun("produces tighter source images than VBAP, useful for precision placement."),
    ]),
    bulletItem([
      boldRun("Ambisonics HOA "),
      textRun("is ideal when feeding an Ambisonics decoder or rotation engine downstream."),
    ]),
    pageBreak(),
  ];
}

// ─── Chapter 7: HRTF Profiles ───────────────────────────────────
function chapter7() {
  return [
    heading1("7. HRTF Profiles"),
    bodyText(
      "OpenSpatialDelay includes 6 HRTF profiles for binaural output. When the output format is set to Binaural, the algorithm dropdown in the header bar is replaced by the HRTF Profile selector."
    ),

    heading2("7.1 Profile Summary"),
    makeTable(
      ["Profile", "SOFA File", "Characteristics"],
      [
        [
          "Simple (Low CPU)",
          "None (Woodworth model)",
          "ITD+ILD model only. No SOFA convolution. Lowest CPU usage.",
        ],
        [
          "Studio Reference",
          "MIT KEMAR Large Pinna",
          "Flat, accurate response. Head radius 0.0875 m.",
        ],
        [
          "Immersive",
          "SADIE II D2 KU100",
          "Neumann KU100 dummy head. Lush, spacious sound with wider ILD.",
        ],
        [
          "Natural",
          "CIPIC Subject 003",
          "Human subject measurement. Subtler, more intimate spatial cues.",
        ],
        [
          "Precise",
          "HUTUBS PP2",
          "Cross-validated, balanced profile for detailed positioning.",
        ],
        [
          "Spatial",
          "Bernschuetz KU100",
          "High-resolution KU100 capture. Exaggerated spatial cues for wide image.",
        ],
      ],
      [2200, 2800, 4360]
    ),

    heading2("7.2 Choosing a Profile"),
    makeTable(
      ["Use Case", "Recommended Profile"],
      [
        ["Low CPU / quick preview", "Simple (Low CPU)"],
        ["Flat, accurate monitoring", "Studio Reference"],
        ["Lush, spacious sound design", "Immersive"],
        ["Intimate vocal/instrument recordings", "Natural"],
        ["Detailed spatial positioning work", "Precise"],
        ["Wide panoramic image", "Spatial"],
      ],
      [4680, 4680]
    ),

    heading2("7.3 Technical Notes"),
    bulletItem([
      boldRun("Partitioned convolution: "),
      textRun("SOFA profiles use partitioned FFT overlap-save convolution with 12 independent convolver pairs (left + right ear), one per delay tap."),
    ]),
    bulletItem([
      boldRun("Double-buffered switching: "),
      textRun("HRTF profile changes use two BinauralRenderer instances. The new profile loads in the background, then swaps atomically with no audio dropout."),
    ]),
    bulletItem([
      boldRun("HRIR updates: "),
      textRun("Triggered when a source position changes by more than approximately 1 degree, using KD-tree nearest-neighbor lookup via libmysofa."),
    ]),
    bulletItem([
      boldRun("Sample rate matching: "),
      textRun("SOFA HRIR data is resampled to match the current DAW session sample rate during loading."),
    ]),
    pageBreak(),
  ];
}

// ─── Chapter 8: Trajectory Animation ────────────────────────────
function chapter8() {
  return [
    heading1("8. Trajectory Animation"),
    bodyText(
      "Each of the 12 delay taps can have an independent trajectory animation that moves the object through 3D space over time. Trajectories are configured per-object in the bottom panel."
    ),

    heading2("8.1 Trajectory Shapes"),
    makeTable(
      ["Shape", "Movement Description"],
      [
        [
          "None",
          "Static position. No animation \u2014 the object stays at its manually set position.",
        ],
        [
          "Spiral",
          "Full 3D spiral: 360-degree azimuth sweep, +/-45 degree elevation sine oscillation, distance pulsing between 0.3 and 1.0.",
        ],
        [
          "Orbit",
          "Simple circular orbit: 360-degree azimuth rotation at constant elevation and distance.",
        ],
        [
          "Bounce",
          "Triangle-wave ping-pong: +/-90 degree azimuth oscillation, +/-30 degree elevation oscillation, constant distance.",
        ],
        [
          "Figure-8",
          "Lissajous curve with 2:1 frequency ratio: +/-90 degree azimuth sine, +/-45 degree elevation at double frequency.",
        ],
        [
          "Random",
          "Deterministic pseudo-random motion using sums of sine oscillators at irrational frequency ratios. Smooth, organic-feeling, fully repeatable.",
        ],
      ],
      [1800, 7560]
    ),

    heading2("8.2 Speed Control"),
    makeTable(
      ["Speed Value", "Behavior"],
      [
        ["0.0", "Frozen \u2014 trajectory holds at its current phase position"],
        ["1.0", "Approximately 1 cycle per second"],
        ["10.0", "Approximately 10 cycles per second"],
      ],
      [2600, 6760]
    ),
    emptyPara(),
    bodyText(
      "The trajectory engine runs at approximately 60 Hz. Phase advances by speed multiplied by dt (1/60 second) per timer tick."
    ),

    heading2("8.3 Base Position"),
    bodyText(
      "When a trajectory shape changes from None to any active shape, the object's current azimuth, elevation, and distance are captured as the base position. All trajectory calculations offset from this base, so the trajectory orbits around wherever the object was when animation started."
    ),

    heading2("8.4 OSC Override Behavior"),
    bodyText(
      "When ADM-OSC messages are actively received for an object, the trajectory animation for that object is paused. OSC position data takes priority. When OSC messages stop arriving (500 ms timeout), the trajectory resumes from where it left off."
    ),
    pageBreak(),
  ];
}

// ─── Chapter 9: ADM-OSC Integration ─────────────────────────────
function chapter9() {
  return [
    heading1("9. ADM-OSC Integration"),
    bodyText(
      "OpenSpatialDelay receives object position data via the ADM-OSC protocol (Audio Definition Model over Open Sound Control), enabling real-time control from external spatializers, DAWs, and automation tools."
    ),

    heading2("9.1 Setup"),
    bulletItem([
      boldRun("Step 1: Enable OSC. "),
      textRun("Click the OSC toggle button in the header bar."),
    ]),
    bulletItem([
      boldRun("Step 2: Set the port. "),
      textRun("Click the port label (default 4002) to edit. Valid range: 1024\u201365535. The port is persisted with the plugin state."),
    ]),
    bulletItem([
      boldRun("Step 3: Configure sender. "),
      textRun("Point your ADM-OSC sender to localhost (or the machine's IP) on the configured UDP port."),
    ]),

    heading2("9.2 Message Reference"),
    bodyText("The following ADM-OSC messages are supported (N = 1-based object index, 1\u201312):"),
    makeTable(
      ["Address Pattern", "Arguments", "Description"],
      [
        ["/adm/obj/N/azim", "float (degrees)", "Set azimuth. Range: -180 to +180."],
        ["/adm/obj/N/elev", "float (degrees)", "Set elevation. Range: -90 to +90."],
        ["/adm/obj/N/dist", "float", "Set normalized distance. Range: 0 to 1."],
        ["/adm/obj/N/aed", "float, float, float", "Set azimuth, elevation, and distance together."],
        ["/adm/obj/N/xyz", "float, float, float", "Set Cartesian position (auto-converted to polar)."],
        ["/adm/obj/N/x", "float", "Set individual X coordinate."],
        ["/adm/obj/N/y", "float", "Set individual Y coordinate."],
        ["/adm/obj/N/z", "float", "Set individual Z coordinate."],
      ],
      [2800, 2400, 4160]
    ),

    heading2("9.3 Cartesian-to-Polar Conversion"),
    bodyText("When Cartesian coordinates are received (/xyz, /x, /y, /z), the conversion follows ITU-R BS.2127-0:"),
    bodyText("azimuth  = atan2(-x, y) * (180 / pi)", {
      runOpts: { font: "Courier New", size: 20 },
    }),
    bodyText("elevation = atan2(z, sqrt(x^2 + y^2)) * (180 / pi)", {
      runOpts: { font: "Courier New", size: 20 },
    }),
    bodyText("distance  = clamp(0, 1, sqrt(x^2 + y^2 + z^2))", {
      runOpts: { font: "Courier New", size: 20 },
    }),

    heading2("9.4 Behavior"),
    bulletItem([
      boldRun("Parameter update: "),
      textRun("Received positions are written to APVTS parameters via setValueNotifyingHost, making them visible to the DAW's automation system and the plugin UI simultaneously."),
    ]),
    bulletItem([
      boldRun("Trajectory override: "),
      textRun("When OSC messages arrive for an object, the trajectory animation for that object is paused. An \"OSC\" label appears on the spatial map beneath the affected object."),
    ]),
    bulletItem([
      boldRun("Timeout: "),
      textRun("If no OSC message is received for an object within 500 ms, the override is released and the trajectory animation resumes."),
    ]),
    bulletItem([
      boldRun("Clamping: "),
      textRun("Received values are clamped before application: azimuth to -180/+180, elevation to -90/+90, distance to 0/1."),
    ]),

    heading2("9.5 Compatible Senders"),
    bulletItem([boldRun("Nuendo "), textRun("(Steinberg) \u2014 native ADM-OSC output")]),
    bulletItem([boldRun("SPAT Revolution "), textRun("(FLUX::) \u2014 ADM-OSC send")]),
    bulletItem([boldRun("IEM StereoEncoder / IEM MultiEncoder "), textRun("\u2014 OSC output")]),
    bulletItem("Custom scripts using the ADM-OSC namespace"),

    heading2("9.6 Test Script"),
    bodyText("A Python test script is included for verifying OSC connectivity:"),
    bodyText("pip install python-osc", {
      runOpts: { font: "Courier New", size: 20 },
    }),
    bodyText("python3 scripts/adm_osc_test.py", {
      runOpts: { font: "Courier New", size: 20 },
    }),
    bodyText("The script sends test position messages to all 12 objects, cycling through various positions."),
    pageBreak(),
  ];
}

// ─── Chapter 10: Presets ────────────────────────────────────────
function chapter10() {
  return [
    heading1("10. Presets"),
    bodyText(
      "OpenSpatialDelay includes 8 factory presets and supports saving/loading user presets as JSON files."
    ),

    heading2("10.1 Factory Presets"),
    makeTable(
      ["Preset", "Taps", "Delay", "Feedback", "Description"],
      [
        ["Default", "4", "500 ms", "30%", "Diagonal cross at -45, +45, -135, +135 degrees. VBAP algorithm."],
        ["Stereo Ping-Pong", "2", "350 ms", "50%", "Hard left/right at -90 and +90 degrees."],
        ["Circle (Quad)", "4", "250 ms", "40%", "Equidistant ring at 0, +90, +180, -90 degrees."],
        ["Surround 5.1", "5", "300 ms", "35%", "Standard 5.1 positions: C, L, R, Ls, Rs."],
        ["Surround 7.1", "7", "250 ms", "35%", "Standard 7.1 positions: C, L, R, Ls, Rs, Lrs, Rrs."],
        ["Atmos 7.1.4", "11", "200 ms", "30%", "7 ear-level + 4 height objects at +45 degree elevation."],
        ["Rising Spiral", "8", "200 ms", "40%", "Spiraling upward with Orbit trajectory, +2 st pitch shift, doppler."],
        ["Falling Cascade", "6", "350 ms", "45%", "Descending elevation at increasing distance, -1 st pitch shift."],
      ],
      [1800, 700, 1000, 1100, 4760]
    ),

    heading2("10.2 User Preset Workflow"),
    heading3("Saving a Preset"),
    bulletItem("Click the Save button in the header bar"),
    bulletItem("Enter a name in the dialog"),
    bulletItem("The preset is saved as a JSON file"),

    heading3("Browsing Presets"),
    bulletItem("Use the ComboBox dropdown in the header bar to see all available presets"),
    bulletItem("Use the Prev and Next arrow buttons to step through presets sequentially"),
    bulletItem("Factory presets appear first in the list, followed by user presets sorted alphabetically"),

    heading2("10.3 File Location"),
    bodyText("User presets are stored at:", { runOpts: { bold: true } }),
    bodyText("~/Library/Application Support/OpenSpatialDelay/Presets/{name}.json", {
      runOpts: { font: "Courier New", size: 20 },
    }),

    heading2("10.4 What Presets Store"),
    bodyText("Presets capture and restore all of the following:"),
    heading3("Global Parameters"),
    bulletItem("Delay Time, Tempo Sync, Note Division, Sync Mode"),
    bulletItem("Feedback"),
    bulletItem("LP Filter, HP Filter"),
    bulletItem("Pitch Shift"),
    bulletItem("Dry/Wet, Input Gain, Output Gain"),
    bulletItem("Air Absorption toggle"),
    bulletItem("Algorithm, HRTF Profile"),

    heading3("Per-Tap Parameters (x12)"),
    bulletItem("Enabled state"),
    bulletItem("Azimuth, Elevation, Distance"),
    bulletItem("Doppler Amount"),
    bulletItem("Trajectory Shape, Trajectory Speed"),

    heading3("Excluded from Presets"),
    bodyText("The following are excluded from presets and persist independently:"),
    bulletItem("Output Format (tied to DAW bus configuration)"),
    bulletItem("ADM-OSC Enabled state"),
    bulletItem("OSC Receive Port"),
    pageBreak(),
  ];
}

// ─── Chapter 11: Workflow Tutorials ─────────────────────────────
function chapter11() {
  return [
    heading1("11. Workflow Tutorials"),

    heading2("11.1 Stereo Ping-Pong Delay"),
    bodyText(
      "Create a classic stereo ping-pong effect with spatial positioning."
    ),
    bulletItem("Load the \"Stereo Ping-Pong\" factory preset, or set up manually:"),
    bulletItem("Enable 2 taps. Set Object 1 azimuth to -90 degrees (left) and Object 2 to +90 degrees (right).", 1),
    bulletItem("Set delay time to 350 ms and feedback to 50%.", 1),
    bulletItem("Set the output format to Stereo or Binaural"),
    bulletItem("Play audio and adjust delay time and feedback to taste"),

    heading2("11.2 Immersive Surround Wash"),
    bodyText(
      "Create a lush surround delay wash using the 5.1 preset."
    ),
    bulletItem("Load the \"Surround 5.1\" factory preset"),
    bulletItem("Set the output format to 5.1 Surround"),
    bulletItem("Increase feedback to 60-70% for a denser wash"),
    bulletItem("Lower the LP Filter to 8000-12000 Hz to darken successive repeats"),
    bulletItem("Enable Air Absorption for distance-dependent high-frequency rolloff"),

    heading2("11.3 Atmos Object Delay"),
    bodyText(
      "Place delay taps in a 7.1.4 Atmos speaker layout with height animation."
    ),
    bulletItem("Load the \"Atmos 7.1.4\" factory preset"),
    bulletItem("Set the output format to 7.1.4 Atmos"),
    bulletItem("Select the height objects (taps 8-11) and enable a trajectory shape such as Orbit or Spiral"),
    bulletItem("Adjust trajectory speed to 0.5-2.0 for gentle overhead motion"),
    bulletItem("Fine-tune delay time and feedback to complement the source material"),

    heading2("11.4 Evolving Texture"),
    bodyText(
      "Build a generative, ever-changing spatial texture."
    ),
    bulletItem("Enable 6-8 taps and spread them across the spatial map"),
    bulletItem("Set different trajectory shapes on each tap (e.g., Random on some, Spiral on others)"),
    bulletItem("Vary trajectory speeds between 0.3 and 3.0 for organic polyrhythmic motion"),
    bulletItem("Add pitch shift (+2 to +5 semitones) for rising harmonics on each feedback cycle"),
    bulletItem("Set feedback to 40-60% and lower the LP Filter for a softening effect over time"),

    heading2("11.5 ADM-OSC Automation"),
    bodyText(
      "Control delay tap positions from an external spatializer or automation tool."
    ),
    bulletItem("Enable the OSC toggle in the header bar and note the port (default 4002)"),
    bulletItem("Configure your external ADM-OSC sender (e.g., Nuendo, SPAT Revolution) to target localhost on the same port"),
    bulletItem("In the external tool, automate object positions. The plugin will receive and display the positions in real time."),
    bulletItem("Objects under OSC control show an \"OSC\" label on the spatial map"),
    bulletItem("To return to manual/trajectory control, stop the external sender. After a 500 ms timeout, trajectories resume."),
    pageBreak(),
  ];
}

// ─── Chapter 12: Troubleshooting ────────────────────────────────
function chapter12() {
  return [
    heading1("12. Troubleshooting"),

    heading2("No Audio Output"),
    bulletItem([
      boldRun("Check output format: "),
      textRun("Ensure the selected output format matches your DAW bus configuration. If the DAW provides 2 channels but a surround format is selected, audio may not route correctly."),
    ]),
    bulletItem([
      boldRun("Check Dry/Wet: "),
      textRun("If Dry/Wet is set to 0%, only the dry signal passes through. Increase to hear the spatial delay effect."),
    ]),
    bulletItem([
      boldRun("Check object state: "),
      textRun("At least one object (delay tap) must be enabled (ON) for wet signal to be produced."),
    ]),

    heading2("Plugin Not Found by DAW"),
    bulletItem("Verify the plugin is in the correct location:"),
    bulletItem("macOS AU: ~/Library/Audio/Plug-Ins/Components/", 1),
    bulletItem("macOS VST3: ~/Library/Audio/Plug-Ins/VST3/", 1),
    bulletItem("Windows VST3: C:\\Program Files\\Common Files\\VST3\\", 1),
    bulletItem("After placing the plugin file, trigger a plugin rescan in your DAW."),

    heading2("Glitchy or Chirpy Audio on Second Feedback Cycle"),
    bodyText(
      "This is a known issue related to DAW bus layout renegotiation. The DAW may renegotiate the bus layout after the plugin reports its channel configuration, causing buffer discontinuities."
    ),
    bulletItem([
      boldRun("Immediate recovery: "),
      textRun("Restart the DAW or reload the plugin to clear the corrupted state."),
    ]),

    heading2("OSC Not Receiving"),
    bulletItem([
      boldRun("Check the port: "),
      textRun("Ensure the configured port (default 4002) is not in use by another application."),
    ]),
    bulletItem([
      boldRun("Check the toggle: "),
      textRun("The OSC toggle in the header bar must be ON (highlighted)."),
    ]),
    bulletItem([
      boldRun("Check the firewall: "),
      textRun("Ensure your system firewall allows incoming UDP on the configured port."),
    ]),
    bulletItem([
      boldRun("Check the namespace: "),
      textRun("The sender must use the /adm/obj/N/ namespace with 1-based object indices (N = 1\u201312)."),
    ]),

    heading2("HRTF Profile Loading Slowly"),
    bodyText(
      "Loading a SOFA profile involves reading and resampling HRIR data. Larger files (e.g., SADIE II D2 KU100 at approximately 35 MB) take longer. Loading is double-buffered \u2014 audio continues through the previous profile without dropout while the new profile loads in the background."
    ),

    heading2("Trajectory Not Moving"),
    bulletItem([
      boldRun("Check the shape: "),
      textRun("If the trajectory shape is set to \"None,\" no animation occurs."),
    ]),
    bulletItem([
      boldRun("Check the speed: "),
      textRun("If the trajectory speed is 0.0, the trajectory is frozen. Increase speed above 0."),
    ]),
    bulletItem([
      boldRun("Check OSC override: "),
      textRun("If ADM-OSC is actively sending position data, trajectories are paused. Disable OSC or stop the sender to resume."),
    ]),

    heading2("Presets Not Saving"),
    bodyText("Verify write permissions on the user preset directory:"),
    bodyText("~/Library/Application Support/OpenSpatialDelay/Presets/", {
      runOpts: { font: "Courier New", size: 20 },
    }),
    bodyText("If the directory does not exist, the plugin creates it on the first save. If creation fails, check that ~/Library/Application Support/ is writable."),

    heading2("Elevation Not Visible on Map"),
    bodyText(
      "Elevation is encoded visually on the 2D spatial map using dot size and opacity (IEM standard). Objects above ear level appear as larger, brighter dots; objects below appear as smaller, dimmer dots. To see the exact elevation in degrees, select an object that has non-zero elevation."
    ),
    pageBreak(),
  ];
}

// ─── Appendix A: Parameter Reference ────────────────────────────
function appendixA() {
  return [
    heading1("Appendix A: Parameter Reference"),
    bodyText(
      "All parameters are exposed through JUCE's AudioProcessorValueTreeState (APVTS) and are fully automatable in any compatible DAW."
    ),

    heading2("A.1 Global Parameters"),
    makeTable(
      ["Parameter ID", "Name", "Type", "Range", "Default", "Unit"],
      [
        ["delayTime", "Delay Time", "Float", "1.0\u20132000.0", "500.0", "ms"],
        ["tempoSync", "Tempo Sync", "Bool", "off/on", "off", "\u2014"],
        ["noteDivision", "Note Division", "Float", "1\u201316", "4", "16th notes"],
        ["syncMode", "Sync Mode", "Choice", "0\u20133", "0", "\u2014"],
        ["feedback", "Feedback", "Float", "0.0\u20131.0", "0.3", "%"],
        ["filterLP", "Low-Pass Filter", "Float", "200\u201320000", "20000", "Hz"],
        ["filterHP", "High-Pass Filter", "Float", "20\u20135000", "20", "Hz"],
        ["pitchShift", "Pitch Shift", "Float", "-24.0\u201324.0", "0.0", "st"],
        ["dryWet", "Dry/Wet", "Float", "0.0\u20131.0", "0.5", "%"],
        ["inputGain", "Input Gain", "Float", "-60.0\u201312.0", "0.0", "dB"],
        ["outputGain", "Output Gain", "Float", "-60.0\u201312.0", "0.0", "dB"],
        ["algorithm", "Algorithm", "Choice", "0\u201310", "0", "\u2014"],
        ["hrtfProfile", "HRTF Profile", "Choice", "0\u20135", "0", "\u2014"],
        ["outputFormat", "Output Format", "Choice", "0\u201320", "0", "\u2014"],
        ["airAbsorption", "Air Absorption", "Bool", "off/on", "off", "\u2014"],
        ["admOscEnabled", "ADM-OSC Enabled", "Bool", "off/on", "off", "\u2014"],
      ],
      [1600, 1600, 900, 1500, 1100, 900]
    ),

    heading2("A.2 Per-Object Parameters (x12)"),
    bodyText("Each of the 12 objects (N = 1\u201312) has the following parameters. The parameter ID uses the pattern object{N}_{paramName}."),
    makeTable(
      ["Parameter ID Pattern", "Name", "Type", "Range", "Default"],
      [
        ["object{N}_enabled", "Enabled", "Bool", "off/on", "on (1\u20134), off (5\u201312)"],
        ["object{N}_time", "Time", "Float", "1.0\u20132000.0 ms", "N * 100 ms"],
        ["object{N}_azimuth", "Azimuth", "Float", "-180.0\u2013180.0", "Varies"],
        ["object{N}_elevation", "Elevation", "Float", "-90.0\u201390.0", "0.0"],
        ["object{N}_distance", "Distance", "Float", "0.0\u20131.0", "0.5"],
        ["object{N}_dopplerAmount", "Doppler Amount", "Float", "0.0\u20131.0", "0.0"],
        ["object{N}_trajectoryShape", "Trajectory Shape", "Choice", "0\u20135", "0"],
        ["object{N}_trajectorySpeed", "Trajectory Speed", "Float", "0.0\u201310.0", "1.0"],
      ],
      [2400, 1800, 900, 2000, 2260]
    ),

    heading2("A.3 Default Azimuth Values"),
    makeTable(
      ["Object", "Default Azimuth", "Object", "Default Azimuth"],
      [
        ["1", "-45.0", "7", "-90.0"],
        ["2", "+45.0", "8", "+180.0"],
        ["3", "-135.0", "9", "-30.0"],
        ["4", "+135.0", "10", "+30.0"],
        ["5", "0.0", "11", "-60.0"],
        ["6", "+90.0", "12", "+60.0"],
      ],
      [1600, 2100, 1600, 2100]
    ),
    emptyPara(),
    bodyText("Objects 1\u20134 form a diagonal cross pattern and are enabled by default. Objects 5\u201312 are disabled by default."),
    pageBreak(),
  ];
}

// ─── Appendix B: Output Formats ─────────────────────────────────
function appendixB() {
  return [
    heading1("Appendix B: Output Formats"),
    bodyText("OpenSpatialDelay supports 21 output formats across 4 categories."),

    heading2("B.1 Binaural and Stereo"),
    makeTable(
      ["Format", "Short Name", "Channels", "LFE", "Height"],
      [
        ["Binaural", "Bin", "2", "No", "No"],
        ["Stereo", "St", "2", "No", "No"],
      ],
      [2000, 1600, 1400, 1000, 1000]
    ),

    heading2("B.2 Surround"),
    makeTable(
      ["Format", "Short Name", "Channels", "LFE", "Height"],
      [
        ["Quadraphonic", "Quad", "4", "No", "No"],
        ["5.0 Surround", "5.0", "5", "No", "No"],
        ["5.1 Surround", "5.1", "6", "Yes", "No"],
        ["7.0 Surround", "7.0", "7", "No", "No"],
        ["5.1.2 Atmos", "5.1.2", "8", "Yes", "Yes"],
        ["7.1 Surround", "7.1", "8", "Yes", "No"],
        ["Octaphonic", "Oct", "8", "No", "No"],
        ["7.0.2", "7.0.2", "9", "No", "Yes"],
        ["5.1.4 Atmos", "5.1.4", "10", "Yes", "Yes"],
        ["7.1.2 Atmos", "7.1.2", "10", "Yes", "Yes"],
        ["7.1.4 Atmos", "7.1.4", "12", "Yes", "Yes"],
        ["7.1.6 Atmos", "7.1.6", "14", "Yes", "Yes"],
        ["9.1.6 Atmos", "9.1.6", "16", "Yes", "Yes"],
      ],
      [2000, 1600, 1400, 1000, 1000]
    ),

    heading2("B.3 Ambisonics (AmbiX ACN/SN3D)"),
    makeTable(
      ["Format", "Short Name", "Channels", "Order"],
      [
        ["1st Order Ambi", "FOA", "4", "1"],
        ["2nd Order Ambi", "SOA", "9", "2"],
        ["3rd Order Ambi", "HOA", "16", "3"],
        ["4th Order Ambi", "4OA", "25", "4"],
        ["5th Order Ambi", "5OA", "36", "5"],
        ["6th Order Ambi", "6OA", "49", "6"],
      ],
      [2400, 1600, 1600, 1400]
    ),

    heading2("B.4 LFE Generation"),
    bodyText("For surround formats that include an LFE channel:"),
    bulletItem("Filter: 2nd-order Butterworth low-pass at 120 Hz"),
    bulletItem("Level: -10 dB (gain factor 0.316)"),
    bulletItem("Source: Derived from the mono sum of all wet delay taps"),
    bulletItem("LFE channel index: Typically channel 3 (JUCE/SMPTE convention: L, R, C, LFE, ...)"),
    pageBreak(),
  ];
}

// ─── Appendix C: ADM-OSC Messages ───────────────────────────────
function appendixC() {
  return [
    heading1("Appendix C: ADM-OSC Message Reference"),
    bodyText("Quick reference for all supported ADM-OSC message types. N is a 1-based object index (1\u201312). Transport: UDP."),
    emptyPara(),
    makeTable(
      ["Address Pattern", "Arguments", "Range / Notes"],
      [
        ["/adm/obj/N/azim", "float (degrees)", "Azimuth: -180 to +180"],
        ["/adm/obj/N/elev", "float (degrees)", "Elevation: -90 to +90"],
        ["/adm/obj/N/dist", "float", "Normalized distance: 0 to 1"],
        ["/adm/obj/N/aed", "float, float, float", "Azimuth, elevation, distance (combined)"],
        ["/adm/obj/N/xyz", "float, float, float", "Cartesian position (auto-converted to polar)"],
        ["/adm/obj/N/x", "float", "Individual X coordinate"],
        ["/adm/obj/N/y", "float", "Individual Y coordinate"],
        ["/adm/obj/N/z", "float", "Individual Z coordinate"],
      ],
      [2800, 2400, 4160]
    ),
  ];
}

// ═══════════════════════════════════════════════════════════════
//  DOCUMENT ASSEMBLY
// ═══════════════════════════════════════════════════════════════

async function main() {
  const doc = new Document({
    styles: {
      default: {
        document: {
          run: {
            font: FONT,
            size: BODY_SIZE,
            color: COLORS.darkGray,
          },
        },
        heading1: {
          run: {
            font: FONT,
            size: H1_SIZE,
            bold: true,
            color: COLORS.darkNavy,
          },
          paragraph: {
            spacing: { before: 360, after: 200 },
            outlineLevel: 0,
          },
        },
        heading2: {
          run: {
            font: FONT,
            size: H2_SIZE,
            bold: true,
            color: COLORS.darkNavy,
          },
          paragraph: {
            spacing: { before: 280, after: 160 },
            outlineLevel: 1,
          },
        },
        heading3: {
          run: {
            font: FONT,
            size: H3_SIZE,
            bold: true,
            color: COLORS.darkGray,
          },
          paragraph: {
            spacing: { before: 200, after: 120 },
            outlineLevel: 2,
          },
        },
      },
    },
    numbering: {
      config: [
        {
          reference: BULLET_REF,
          levels: [
            {
              level: 0,
              format: LevelFormat.BULLET,
              text: "\u2022",
              alignment: AlignmentType.LEFT,
              style: {
                paragraph: {
                  indent: { left: 720, hanging: 360 },
                },
                run: {
                  font: FONT,
                  size: BODY_SIZE,
                },
              },
            },
            {
              level: 1,
              format: LevelFormat.BULLET,
              text: "\u2013",
              alignment: AlignmentType.LEFT,
              style: {
                paragraph: {
                  indent: { left: 1440, hanging: 360 },
                },
                run: {
                  font: FONT,
                  size: BODY_SIZE,
                },
              },
            },
          ],
        },
      ],
    },
    sections: [
      {
        properties: {
          page: {
            size: {
              width: 12240,
              height: 15840,
            },
            margin: {
              top: 1440,
              right: 1440,
              bottom: 1440,
              left: 1440,
            },
          },
        },
        headers: {
          default: new Header({
            children: [
              new Paragraph({
                alignment: AlignmentType.RIGHT,
                children: [
                  new TextRun({
                    text: "OpenSpatialDelay v0.6 \u2014 User Manual",
                    font: FONT,
                    size: SMALL_SIZE,
                    color: COLORS.lightGray,
                  }),
                ],
              }),
            ],
          }),
        },
        footers: {
          default: new Footer({
            children: [
              new Paragraph({
                alignment: AlignmentType.CENTER,
                children: [
                  new TextRun({
                    font: FONT,
                    size: SMALL_SIZE,
                    color: COLORS.lightGray,
                    children: [PageNumber.CURRENT],
                  }),
                ],
              }),
            ],
          }),
        },
        children: [
          ...coverPage(),
          ...tocPage(),
          ...chapter1(),
          ...chapter2(),
          ...chapter3(),
          ...chapter4(),
          ...chapter5(),
          ...chapter6(),
          ...chapter7(),
          ...chapter8(),
          ...chapter9(),
          ...chapter10(),
          ...chapter11(),
          ...chapter12(),
          ...appendixA(),
          ...appendixB(),
          ...appendixC(),
        ],
      },
    ],
  });

  const buffer = await Packer.toBuffer(doc);
  const outputPath = __dirname + "/OpenSpatialDelay_User_Manual_v0.6.docx";
  fs.writeFileSync(outputPath, buffer);
  console.log("Manual generated:", outputPath);
  console.log("File size:", (buffer.length / 1024).toFixed(1), "KB");
}

main().catch((err) => {
  console.error("Error generating manual:", err);
  process.exit(1);
});
