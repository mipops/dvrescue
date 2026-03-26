# DVRescue Issue & PR Analysis

> Generated: 2026-03-26 | Source: [mipops/dvrescue](https://github.com/mipops/dvrescue) | 131 open issues, 3 open PRs

---

## 1. Critical Bugs (Crashes, Data Loss, Core Functionality Broken)

| # | Title | Platform | Complexity | Notes |
|---|-------|----------|------------|-------|
| [#993](https://github.com/mipops/dvrescue/issues/993) | **Segmentation fault in dvpackager** | macOS (M1) | Medium | Crash at line 1326 of dvpackager. dvrescue crashes before generating XML output. Reproducible on Homebrew 24.07. Multiple users affected. |
| [#987](https://github.com/mipops/dvrescue/issues/987) | **Out of memory with 24.07** | Linux | High | `std::bad_alloc` during `--rewind-count 2` capture. Unbounded vector growth in `dv_merge_private::AddFrameAnalysis()`. Also triggers segfault after MediaInfoLib upgrade. Core C++ memory management issue. |
| [#989](https://github.com/mipops/dvrescue/issues/989) | **DVRescue crashes when recording deck is stopped** | macOS (M2) | High | Crash during multi-stream capture (4 inputs) when user stops deck after stopping software. Reproducible across multiple Sony DSR-1500A decks. |
| [#388](https://github.com/mipops/dvrescue/issues/388) | **Null-filled blocks crash dvrescue** | All | Medium | Segfault when processing DV files with null-filled blocks. Assigned to JeromeMartinez. Test file available. Open since Nov 2021. |
| [#615](https://github.com/mipops/dvrescue/issues/615) | **Catch control-c to finalize XML** | All | Medium | Ctrl-C during capture leaves XML in incomplete/invalid state, causing downstream failures. |
| [#996](https://github.com/mipops/dvrescue/issues/996) | **FireWire no longer supported in macOS Tahoe** | **macOS only** | High | Apple removed FireWire drivers in macOS Tahoe 26.2. Breaks all FireWire-based capture on latest macOS. ASFireWire project suggested as potential workaround. Affects all macOS users who upgrade. |
| [#636](https://github.com/mipops/dvrescue/issues/636) | **Loss of deck control via CLI** | All | Medium | Deck control lost during CLI capture operations. |
| [#936](https://github.com/mipops/dvrescue/issues/936) | **Capture fails/stuck on timecode jump with rewind** | All | High | Capture hangs when rewind encounters timecode discontinuities (overwritten tapes). Drops frames and stops capturing without exiting. |
| [#982](https://github.com/mipops/dvrescue/issues/982) | **Analyses - segment error** | Unknown | Medium | Segmentation error during analysis. |

---

## 2. Minor Bugs (Incorrect Behavior, Non-Crash)

| # | Title | Platform | Complexity | Notes |
|---|-------|----------|------------|-------|
| [#990](https://github.com/mipops/dvrescue/issues/990) | **Invalid characters in filename break dvplay, dvpackager, dvmerge** | All | Low | Ampersand (&) and special chars in filenames cause downstream tool failures. Needs input validation/sanitization. |
| [#930](https://github.com/mipops/dvrescue/issues/930) | **Frame preview on Merge Summary not working on Windows** | Windows | Medium | Clicking file selection hyperlink shows empty modal. dvplay command succeeds but GUI fails to display output. |
| [#943](https://github.com/mipops/dvrescue/issues/943) | **dvpackager does not write split .dv files** | Unknown | Medium | dvpackager fails to produce expected split output files. |
| [#927](https://github.com/mipops/dvrescue/issues/927) | **Packing error: audio-video-duration-mismatch** | All | Medium | Audio-video duration mismatch during packaging. |
| [#672](https://github.com/mipops/dvrescue/issues/672) | **Distorted sound in Analysis window** | All | Medium | Audio playback distortion in GUI analysis view. |
| [#671](https://github.com/mipops/dvrescue/issues/671) | **Recording rate change not flagged in dvanalysis** | All | Low | Rate changes between recordings on same tape not detected/reported. |
| [#783](https://github.com/mipops/dvrescue/issues/783) | **Ctrl-C on capture is inconsistent** | All | Low | Inconsistent behavior when interrupting capture with Ctrl-C. |
| [#739](https://github.com/mipops/dvrescue/issues/739) | **Inconsistency between daily build CLI and embedded CLI in GUI** | All | Medium | Version mismatch between standalone CLI and the one bundled in the GUI. |
| [#608](https://github.com/mipops/dvrescue/issues/608) | **Capture scenario causing CSV misalignment** | All | Medium | CSV output columns become misaligned under certain capture conditions. |
| [#572](https://github.com/mipops/dvrescue/issues/572) | **Fix issue when dvrescue XML is present but contains no frames** | All | Low | Edge case: empty XML file causes failures in downstream tools. |
| [#373](https://github.com/mipops/dvrescue/issues/373) | **dvanalysis GUI table/graph navigation issues** | All | Low | UI navigation problems in analysis display. |
| [#209](https://github.com/mipops/dvrescue/issues/209) | **Audio errors do not properly merge** | All | Medium | dvmerge doesn't correctly handle audio error frames during merge. |
| [#353](https://github.com/mipops/dvrescue/issues/353) | **Incoherent audio characteristics** | All | Medium | Audio characteristics reported inconsistently. |
| [#352](https://github.com/mipops/dvrescue/issues/352) | **Unreported audio error** | All | Low | Certain audio errors not surfaced in output. |
| [#974](https://github.com/mipops/dvrescue/issues/974) | **dvsampler cannot find a font to use on Linux** | Linux | Low | Missing font fallback on Linux. Fixed by PR #978. |
| [#997](https://github.com/mipops/dvrescue/issues/997) | **Sony DSR-1800P not detected** | Windows | Low | Specific deck model not recognized. Other Sony models work fine. Likely device-specific compatibility issue. |
| [#994](https://github.com/mipops/dvrescue/issues/994) | **How to capture with dvrescue CLI and rewind-count** | All | Low | User confusion about CLI usage — partly a documentation gap. |
| [#729](https://github.com/mipops/dvrescue/issues/729) | **DV Deck doesn't show up in DVRescue GUI** | Unknown | Medium | Deck detected by OS but not by DVRescue GUI. |
| [#726](https://github.com/mipops/dvrescue/issues/726) | **Error when executing dvpackager** | All | Medium | MOV/ffmpeg-related packaging error. |
| [#620](https://github.com/mipops/dvrescue/issues/620) | **Unable to Package to .mov using Mac** | **macOS** | Medium | MOV packaging fails on macOS. Marked as duplicate. |
| [#668](https://github.com/mipops/dvrescue/issues/668) | **Capture not working on Linux** | Linux | Medium | Capture functionality broken on Linux. |
| [#935](https://github.com/mipops/dvrescue/issues/935) | **dvrescue only able to see one DV device - Ubuntu** | Linux | Medium | Multiple devices not detected on Ubuntu. |
| [#445](https://github.com/mipops/dvrescue/issues/445) | **Capture not detecting camcorder on Windows** | Windows | Medium | Camcorder not detected on Windows platform. |
| [#725](https://github.com/mipops/dvrescue/issues/725) | **Capturing via GUI on systems other than macOS** | Windows | Medium | GUI capture limited to macOS. Marked as duplicate. |
| [#724](https://github.com/mipops/dvrescue/issues/724) | **dvcapture not capturing** | Unknown | Medium | Capture fails silently. |

---

## 3. Feature Requests & Enhancements

| # | Title | Platform | Complexity | Notes |
|---|-------|----------|------------|-------|
| [#929](https://github.com/mipops/dvrescue/issues/929) | **Better support for timecode incoherency/missing subcode data** | All | High | Improve handling of non-sequential or missing timecode data during analysis. |
| [#938](https://github.com/mipops/dvrescue/issues/938) | **Resync/resample audio when packaging DV files** | All | High | New feature: audio resampling during packaging to fix drift. |
| [#945](https://github.com/mipops/dvrescue/issues/945) | **dvcapture GUI: option to set default directory** | All | Low | Simple UI enhancement for default save location. |
| [#980](https://github.com/mipops/dvrescue/issues/980) | **Option to exclude Blackmagic Decklink devices in capture window** | All | Low | Filter out non-DV capture devices from device list. |
| [#981](https://github.com/mipops/dvrescue/issues/981) | **Methods for removing error/bad portions during packaging** | All | High | Allow users to strip error-heavy segments when packaging. |
| [#914](https://github.com/mipops/dvrescue/issues/914) | **Option to output frames during rewind/merge** | All | Medium | Output intermediate frames during rewind operations. |
| [#913](https://github.com/mipops/dvrescue/issues/913) | **Show icon when rewind/merge is active in capture** | All | Low | Visual indicator for rewind/merge status. |
| [#915](https://github.com/mipops/dvrescue/issues/915) | **Add examples to dvrescue -h** | All | Low | Documentation improvement for CLI help output. |
| [#764](https://github.com/mipops/dvrescue/issues/764) | **Merge interface requests** | All | Medium | UI improvements for the merge workflow. |
| [#752](https://github.com/mipops/dvrescue/issues/752) | **Add prefix/suffix options for file names** | All | Low | Custom naming patterns for output files. |
| [#737](https://github.com/mipops/dvrescue/issues/737) | **Support for status, type, timer1/2, ltc_tc_ub, vitc_tc_ub** | All | High | Additional metadata fields in dvrescue output. Milestone: 24.02. |
| [#736](https://github.com/mipops/dvrescue/issues/736) | **Need sony9pin control responses for dvrescue** | All | High | Sony 9-pin protocol integration for deck control. |
| [#784](https://github.com/mipops/dvrescue/issues/784) | **MKV Decklink notes** | All | Medium | Decklink-related MKV support. |
| [#771](https://github.com/mipops/dvrescue/issues/771) | **dvrescue vs sony9pin deckcontrol speed** | All | Medium | Performance comparison/improvement for deck control. |
| [#426](https://github.com/mipops/dvrescue/issues/426) | **Option to enable/disable/adjust rewind/merge during capture** | All | Medium | User-configurable rewind/merge parameters. |
| [#416](https://github.com/mipops/dvrescue/issues/416) | **dvrescue --merge concatenated files** | All | Medium | Merge already-concatenated files. |
| [#433](https://github.com/mipops/dvrescue/issues/433) | **GUI: touchpad scrolling** | All | Low | Touchpad scroll support in GUI. |
| [#381](https://github.com/mipops/dvrescue/issues/381) | **Add version variable to dvpackager** | All | Low | Version tracking in dvpackager script. |
| [#389](https://github.com/mipops/dvrescue/issues/389) | **merge-log and stderr** | All | Low | Improve merge logging output. |
| [#357](https://github.com/mipops/dvrescue/issues/357) | **Report invalid stype** | All | Low | Flag invalid DV stream types. |
| [#356](https://github.com/mipops/dvrescue/issues/356) | **Optional file suffixes for split output** | All | Low | Customizable suffixes when splitting files. |
| [#624](https://github.com/mipops/dvrescue/issues/624) | **Segmenting rules: rec start vs rec stop markers** | All | Medium | More granular control over segmentation logic. |
| [#616](https://github.com/mipops/dvrescue/issues/616) | **Support option to override media/@ref and @fromCapture** | All | Low | Marked as "resolved?" — may need verification and closing. |
| [#610](https://github.com/mipops/dvrescue/issues/610) | **Testing merge-ignore filters** | All | Medium | Merge filtering improvements. |
| [#607](https://github.com/mipops/dvrescue/issues/607) | **Document dvrescue CSV** | All | Low | Documentation for CSV output format. |
| [#204](https://github.com/mipops/dvrescue/issues/204) | **frames/@scan_type defined but not implemented** | All | Medium | XML schema field exists but has no implementation. Milestone: xml 1.2. |
| [#128](https://github.com/mipops/dvrescue/issues/128) | **Flexible handling of arbitrary bit inconsistency** | All | High | Core analysis improvement. Milestone: xml 1.2. |
| [#120](https://github.com/mipops/dvrescue/issues/120) | **Revise the WebVTT output** | All | Low | Improve subtitle/chapter output format. |
| [#96](https://github.com/mipops/dvrescue/issues/96) | **dvpackager - accept folder input for -u** | All | Low | Allow directory as input to dvpackager. |
| [#53](https://github.com/mipops/dvrescue/issues/53) | **Is HDV within scope of the project?** | All | N/A | Labeled `wontfix`. Scope question — not actionable. |
| [#937](https://github.com/mipops/dvrescue/issues/937) | **Best Mac OS for DVRescue** | **macOS** | N/A | User question, not a feature request. Relevant given #996 (Tahoe FireWire removal). |

---

## 4. Open Pull Requests (Stale / Ready to Merge)

| PR # | Title | Author | Age | Status | Complexity | Ready? |
|------|-------|--------|-----|--------|------------|--------|
| [#975](https://github.com/mipops/dvrescue/pull/975) | **fix: dvplay fails when only provided an input file** | acuteaangle | ~1 year (Mar 2025) | No reviews, no CI feedback | Low | **Yes** — Small, focused fix for uninitialized `${FILTER}` variable. Resolves #972. Should be straightforward to review and merge. |
| [#978](https://github.com/mipops/dvrescue/pull/978) | **fix(dvsampler): use Liberation font on Linux** | acuteaangle | ~1 year (Mar 2025) | No reviews, no CI feedback | Low | **Yes** — Ports existing dvplay font fallback logic to dvsampler. Resolves #974. Single commit, minimal risk. |
| [#985](https://github.com/mipops/dvrescue/pull/985) | **Bump rexml from 3.3.9 to 3.4.2 in /docs** | dependabot | ~6 months (Sep 2025) | No reviews | Low | **Yes** — Automated dependency bump for docs site. Standard security/maintenance update. |

---

## Summary & Recommendations

### Highest Priority Actions
1. **Fix OOM crash (#987)** — Unbounded vector growth in C++ frame analysis is a memory safety issue. Affects long captures with rewind.
2. **Fix segfault in dvpackager (#993)** — Multiple users blocked; dvpackager unusable on M1 Macs with v24.07.
3. **Fix crash on deck stop (#989)** — Multi-stream capture crash on macOS M2 during normal workflow.
4. **Address macOS Tahoe FireWire removal (#996)** — Existential threat to macOS capture workflow. Investigate ASFireWire integration.
5. **Fix null-block segfault (#388)** — Open 4+ years with test file available. Should be fixable.

### Quick Wins (merge these PRs)
- PR #975, #978, #985 are all small, focused fixes with no reviews but low risk. Merging them would close 2 issues (#972, #974) immediately.

### macOS-Specific Items
- #996 (Tahoe FireWire removal) — **Critical**, affects all macOS users on latest OS
- #993 (dvpackager segfault) — Reported on M1 Mac
- #989 (crash on deck stop) — Reported on M2 Mac
- #620 (MOV packaging fails) — macOS specific
- #937 (best macOS version) — informational, relevant to #996
