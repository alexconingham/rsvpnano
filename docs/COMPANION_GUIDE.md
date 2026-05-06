# Bookworm — reader manual

**This document is not part of the device.** It is a standalone booklet for people who use RSVP Nano with Bookworm: read it on a phone or computer, **print** it, or **export to PDF** (for example open this file in a Markdown viewer or editor and print / “Save as PDF”). Nothing here is shown inside the app itself.

It explains the desk companion: what you see, how it behaves, and what the **Settings → Bookworm** options do.

---

## Quick start

- After boot you normally land on the **companion** screen (clock, pet, hunger/boredom bars).
- Open the **menu** (device button / gesture as usual) and use **Resume** to load your saved book, or **Library** for another title.
- **Desk** in the menu saves your place, unloads the book from RAM, and returns to the companion.

---

## Needs: hunger and boredom

Two bars (labeled **H** and **B**) range from empty to full (0–100%).

| What happens | Hunger (H) | Boredom (B) |
|--------------|------------|-------------|
| **Over time** | Slowly rises | Slowly rises |
| **While you read** (playing RSVP) | Drops as you read | Drops as you read |
| **Feed** (left zone on the companion strip) | Drops a lot | Unchanged |
| **Play** (middle zone) | Unchanged | Drops a lot |
| **Pet** (right zone) | Drops a bit | Drops a bit |

If either bar stays very high for a long time, the pet can feel **unwell** (see *Sickness*). A red **!** near the clock appears when a need is very high — a gentle “check on me” nudge, not an alarm.

---

## Tap the pet (boop)

Tap the **upper area** of the companion (above the desk buttons, on the creature). That is a light **boop**: small relief for both needs, a quick flash, and a short fun message. It does **not** replace Feed / Play / Pet for big changes.

---

## Desk strip: Feed, Play, Pet

The bottom of the companion has three tap zones (left → right): **Feed**, **Play**, **Pet**.

- Short **toasts** (on-screen messages) rotate so repeated taps do not always show the same text.
- **Feed spam**: tapping **Feed** several times within a few seconds can trigger **“Too full!”** — hunger then rises **faster** for a while (**overfull** debuff). Space feeds out if you want to avoid that.

---

## Expressions and mood

The procedural creature’s eyes and face react to **hunger** and **boredom** (tired, stressed, content, etc.). When the pet is **sick**, the face looks more unwell. **Hibernate** (in settings) hides the sprite and pauses the simulation.

---

## Age (days)

If the device has learned the correct **date/time** (usually after Wi‑Fi and SNTP, e.g. when using firmware update), the companion can show a small **day count** (e.g. `12d`) next to the level. That is **days since hatch** in UTC. If time is not set yet, age may be hidden until the clock is valid.

---

## Care score and the gold “+”

Reading with needs effects on, and using **Feed / Play / Pet / boop**, slowly raises a hidden **care score**. When it is high enough, a small yellow **+** appears beside the level/age line — a cosmetic “bond” accent, not a separate meter.

---

## Sickness

If **both** hunger **and** boredom stay high together for a sustained time, **sick** buildup can cross a threshold:

- Mood text may show something like **“Not well…”**
- The creature looks sicker
- **Desk care** (Feed, Play, Pet, boop) **reduces** sick buildup and helps recovery
- Getting needs back to healthier levels also helps clear the streak over time

With **Needs sim** turned **Off** in settings, sickness mechanics do not run (see below).

---

## Settings → Bookworm

| Setting | What it does |
|---------|----------------|
| **Hibernate** | **On**: simulation paused, creature hidden (resting). Words you read can still count toward totals/evolution depending on other settings. **Off**: normal companion. |
| **Needs sim** | **On** (default): hunger/boredom drift, sickness, overfull, care score updates, desk actions change needs, attention **!**. **Off**: needs **freeze** in the background; you see a calm “neutral” face and bars; desk/boop only show toasts and flash — **no** need changes, no sickness/care updates, no **!**. Turning **Off** clears active sick/overfull buildup. |
| **Evolution** | **On** (default): reading advances **stage** (appearance ornaments) from your lifetime word count. **Off**: creature is always drawn as **stage 0**; word count still increases, and turning **Evolution** back **On** **catch-up**s the stage from saved words. |
| **Boot: companion / Boot: book** | Chooses the first screen after boot. |

Dev firmware may add test entries (regenerate pet, force evolution).

---

## Data and privacy

- Companion save data lives in NVS namespace **`bworm`** (separate from general **`rsvp`** preferences).
- Your **book** progress and WPM still use the usual reading prefs.

---

## Troubleshooting

- **Clock shows `--:--`**: time not synced yet; connect Wi‑Fi so SNTP can run, or use a path that sets time. Age may not appear until then.
- **Too frantic**: turn **Needs sim** **Off** for a decorative pet, or use **Hibernate** to rest without deleting anything.
- **Want evolution ornaments back**: **Settings → Bookworm → Evolution: On** (stage updates from saved word count).

Firmware updates, books, and OTA are covered in the main RSVP Nano project documentation for builders and maintainers (for example the repository README if you obtained this file from source control).
