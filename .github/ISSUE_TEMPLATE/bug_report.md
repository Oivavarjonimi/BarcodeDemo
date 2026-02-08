---
name: Bug report
about: Report a problem in BarcodeDemo
title: "[Bug] "
labels: bug
---

## Description
What happened?

## Steps to reproduce
1.
2.
3.

## Expected behavior
What did you expect to happen?

## Environment
- OS:
- Qt version:
- Build type: Debug/Release
- Decoder backend: ZXing/ZBar
- Phone input: DroidCam? (IP/port)

## Logs / screenshots
Paste relevant output here.

------------------------------------------------------------------------------------------------------------------------------------------------------------
name: Bug report
about: Report a functional bug or unexpected behavior
title: "[BUG] Decode uses stale image instead of phone live snapshot"
labels: bug
assignees: 'KLe'
---

## Bug description
When using **Snap (Live)** with DroidCam phone input, pressing **Decode** does not decode the image currently shown from the phone camera.  
Instead, decoding always returns the data from the **first loaded image** (e.g. an earlier opened QR code), even though the preview shows a different image.

---

## Steps to reproduce
1. Start the application
2. Open a QR image from disk (or decode any image once)
3. Press **Snap** to start phone live preview (DroidCam)
4. Point phone camera to a different QR code
5. Press **Snap / Freeze** to capture phone image
6. Press **Decode**

---

## Expected behavior
- Decode should process the **latest image shown in preview**
- When using phone input, Decode should use the **frozen phone snapshot**
- Results list should reflect the currently decoded barcode only

---

## Actual behavior
- Decode uses a stale image (usually the first loaded image)
- Results list shows old decoded data
- Phone image is shown in preview but not used for decoding

---

## Root cause analysis
The issue is caused by incorrect image source selection in the decode pipeline:

- `Decode` uses an outdated `QImage` (`m_snapshotImage` or old `m_currentImage`)
- Phone live frames update the preview but do not update the image used by Decode
- Snapshot logic does not overwrite the previously decoded image

---

## Proposed fix
- Treat **Snap** as a toggle:
  - First press: start phone live preview
  - Second press: freeze current frame into `m_snapshotImage`
- Always decode:
  - `m_snapshotImage` if present
  - otherwise `m_currentImage`
- Clear previous decode results before decoding
- Ensure phone MJPEG frames update `m_currentImage`

---

## Environment
- OS: Ubuntu / WSL2 (Windows 11)
- Qt: Qt 6.x
- Compiler: GCC 13
- Camera input: DroidCam (HTTP MJPEG stream)
- Decoder backend: ZXing (default), ZBar optional

---

## Additional notes
This bug affects usability when using a phone camera as an external barcode scanner.  
Fixing it significantly improves real-world usage and demo reliability.
