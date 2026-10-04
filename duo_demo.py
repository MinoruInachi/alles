# duo_demo.py
# A short piece for two Alles synths (e.g. two Atom VoiceS3Rs), placed left and right of the listener.
#
#   python3 duo_demo.py               # play the whole piece
#   python3 duo_demo.py --bpm 100     # slower
#   python3 duo_demo.py --volume 2    # set the synths' volume first (a reset puts it back to 1)
#
# Every note is sent with a host time a little ahead of when it should sound, so the two synths
# play from their own clocks and stay together no matter how the packets arrive.
#
# Sections:
#   1. Roll call       - each synth introduces itself, left then right
#   2. Call & response - a marimba phrase on the left, answered on the right
#   3. Ping-pong       - an arpeggio bouncing between the two, over a pad on both
#   4. Split band      - left: drums + lead, right: bass + pad
#   5. Swap            - the parts change sides
#   6. Ending          - one chord split across both, low on the right, high on the left
import argparse
import time
import alles
from alles import amy

LEFT, RIGHT, BOTH = 0, 1, None

# Synths, set up the same on both units. Notes are then addressed to one unit with client.
MARIMBA, PLUCK, PAD, BASS, DRUMS = 1, 2, 3, 4, 10
SYNTHS = [
    # synth, patch, voices
    (MARIMBA, 149, 3),  # DX7 MARIMBA
    (PLUCK, 52, 2),     # Juno A75 Pluck Bell
    (PAD, 47, 3),       # Juno A68 Synth Pad
    (BASS, 36, 1),      # Juno A55 Synth Bass II
    (DRUMS, 258, 1),    # General MIDI drum kit from the built in 808 samples
]
KICK, SNARE, CLAP, CLOSED_HAT, OPEN_HAT, COWBELL = 36, 38, 39, 42, 46, 56

# Am F C G, one bar each
PROGRESSION = [
    {'root': 45, 'pad': [57, 60, 64], 'arp': [69, 72, 76, 81]},  # Am
    {'root': 41, 'pad': [57, 60, 65], 'arp': [65, 69, 72, 77]},  # F
    {'root': 48, 'pad': [55, 60, 64], 'arp': [67, 72, 76, 79]},  # C
    {'root': 43, 'pad': [55, 59, 62], 'arp': [67, 71, 74, 79]},  # G
]

# (beat, note, beats) over two bars
CALL_1 = [(0, 76, 1), (1, 72, .5), (1.5, 74, .5), (2, 76, 1), (3, 69, 1), (4, 77, 1.5), (5.5, 76, .5), (6, 74, 1), (7, 72, 1)]
ANSWER_1 = [(0, 72, 1), (1, 76, .5), (1.5, 79, .5), (2, 84, 1.5), (3.5, 83, .5), (4, 79, 1), (5, 74, 1), (6, 71, 1), (7, 74, 1)]
CALL_2 = [(0, 81, 1.5), (1.5, 79, .5), (2, 76, 1), (3, 72, 1), (4, 77, 1), (5, 76, .5), (5.5, 74, .5), (6, 72, 2)]
ANSWER_2 = [(0, 76, 1), (1, 79, 1), (2, 84, 1), (3, 79, 1), (4, 83, 1), (5, 79, 1), (6, 74, 1), (7, 71, 1)]


class Score:
    """A list of timed AMY messages, each for one unit or both."""
    def __init__(self, bpm):
        self.beat_ms = 60000.0 / bpm
        self.events = []  # (ms, order, message)

    def at(self, beat, client=BOTH, **kwargs):
        if client is not None:
            kwargs['client'] = client
        self.events.append((beat * self.beat_ms, len(self.events), amy.message(**kwargs)))

    def note(self, beat, synth, note, beats, vel=0.8, client=BOTH):
        self.at(beat, client, synth=synth, note=note, vel=vel)
        # Leave a little gap so a repeated note retriggers
        self.at(beat + beats * 0.9, client, synth=synth, note=note, vel=0)

    def phrase(self, beat, synth, notes, vel=0.8, client=BOTH, transpose=0):
        for (b, n, beats) in notes:
            self.note(beat + b, synth, n + transpose, beats, vel=vel, client=client)

    def hit(self, beat, drum, vel=1.0, client=BOTH):
        self.at(beat, client, synth=DRUMS, note=drum, vel=vel)


def drums(s, beat, client, bars=1, fill=False):
    for bar in range(bars):
        b = beat + bar * 4
        for k in (0, 1.5, 2):
            s.hit(b + k, KICK, 1.0, client)
        for k in (1, 3):
            s.hit(b + k, SNARE, 0.8, client)
        for k in range(8):
            s.hit(b + k * 0.5, OPEN_HAT if k == 7 else CLOSED_HAT, 0.5 if k % 2 else 0.7, client)
        if fill and bar == bars - 1:
            for k in (3, 3.25, 3.5, 3.75):
                s.hit(b + k, CLAP, 0.6, client)


def bassline(s, beat, client, bars=4):
    for bar in range(bars):
        root = PROGRESSION[bar % 4]['root']
        b = beat + bar * 4
        for (k, n, beats) in [(0, root, .75), (1.5, root, .4), (2, root + 12, .4), (3, root, .4), (3.5, root + 7, .4)]:
            s.note(b + k, BASS, n, beats, vel=0.9, client=client)


def pads(s, beat, client, bars=4, vel=0.4):
    for bar in range(bars):
        for n in PROGRESSION[bar % 4]['pad']:
            s.note(beat + bar * 4, PAD, n, 4, vel=vel, client=client)


def compose(bpm):
    s = Score(bpm)
    beat = 0

    # 1. Roll call: left, then right, then both together
    print_at = []
    print_at.append((beat, 'Roll call'))
    for i, n in enumerate([57, 60, 64, 69]):
        s.note(beat + i * 0.5, MARIMBA, n, 0.5, client=LEFT)
    for i, n in enumerate([64, 69, 72, 76]):
        s.note(beat + 2 + i * 0.5, MARIMBA, n, 0.5, client=RIGHT)
    s.hit(beat + 4, COWBELL, 0.8, LEFT)
    s.hit(beat + 4.5, COWBELL, 0.8, RIGHT)
    s.note(beat + 6, MARIMBA, 69, 2, client=BOTH)
    beat += 8

    # 2. Call & response: phrases trade sides every two bars
    print_at.append((beat, 'Call & response'))
    s.phrase(beat, MARIMBA, CALL_1, client=LEFT)
    s.phrase(beat + 8, MARIMBA, ANSWER_1, client=RIGHT)
    s.phrase(beat + 16, MARIMBA, CALL_2, client=LEFT)
    s.phrase(beat + 24, MARIMBA, ANSWER_2, client=RIGHT)
    beat += 32

    # 3. Ping-pong: eighth notes alternate sides, the pad plays on both in sync
    print_at.append((beat, 'Ping-pong'))
    pattern = [0, 1, 2, 3, 2, 1, 2, 3]
    for bar in range(8):
        arp = PROGRESSION[bar % 4]['arp']
        for k, idx in enumerate(pattern):
            s.note(beat + bar * 4 + k * 0.5, PLUCK, arp[idx], 0.5, vel=0.7, client=LEFT if k % 2 == 0 else RIGHT)
    pads(s, beat, BOTH, bars=8, vel=0.3)
    beat += 32

    # 4. Split band: left plays drums and the tune, right plays bass and pad
    print_at.append((beat, 'Split band'))
    drums(s, beat, LEFT, bars=8, fill=True)
    s.phrase(beat, MARIMBA, CALL_1 + [(8 + b, n, d) for (b, n, d) in ANSWER_1], client=LEFT)
    s.phrase(beat + 16, MARIMBA, CALL_2 + [(8 + b, n, d) for (b, n, d) in ANSWER_2], client=LEFT)
    bassline(s, beat, RIGHT, bars=8)
    pads(s, beat, RIGHT, bars=8)
    beat += 32

    # 5. Swap: the same band, sides changed
    print_at.append((beat, 'Swap'))
    drums(s, beat, RIGHT, bars=4, fill=True)
    s.phrase(beat, MARIMBA, CALL_2 + [(8 + b, n, d) for (b, n, d) in ANSWER_1], client=RIGHT, transpose=12)
    bassline(s, beat, LEFT, bars=4)
    pads(s, beat, LEFT, bars=4)
    beat += 16

    # 6. Ending: an A minor chord split across the two, with a cymbal-ish open hat on both
    print_at.append((beat, 'Ending'))
    for n in [33, 45, 52]:
        s.note(beat, BASS if n == 33 else PAD, n, 6, vel=0.8, client=RIGHT)
    for n in [69, 72, 76, 81]:
        s.note(beat, PAD if n < 81 else MARIMBA, n, 6, vel=0.5, client=LEFT)
    s.hit(beat, KICK, 1.0, BOTH)
    s.hit(beat, OPEN_HAT, 0.8, BOTH)
    beat += 8

    return s, print_at, beat


def setup(volume=None):
    # The plain alles.send prefixes the current host time, which is what we want for setup
    alles.send(reset=amy.RESET_ALL_OSCS + amy.RESET_SEQUENCER)
    # The reset plays after the synths' latency, but a synth's patch loads as soon as it arrives.
    # So wait for the reset to happen first, or it would wipe the synths we set up next.
    time.sleep(alles.ALLES_LATENCY_MS / 1000.0 + 0.3)
    if volume is not None:
        alles.send(volume=volume)
    for (synth, patch, voices) in SYNTHS:
        alles.send(synth=synth, patch=patch, num_voices=voices)
        time.sleep(0.1)


def play(score, sections, total_beats, lead_ms=500, lookahead_ms=250):
    events = sorted(score.events)
    start = amy.millis() + lead_ms
    sections = list(sections)
    for (ms, _, message) in events:
        when = start + int(ms)
        # Send each event a little before it is due; the synth adds its own latency on top
        wait = (when - lookahead_ms - amy.millis()) / 1000.0
        if wait > 0:
            time.sleep(wait)
        while sections and sections[0][0] * score.beat_ms <= ms:
            print('%5.1fs  %s' % (ms / 1000.0, sections.pop(0)[1]))
        alles.transmit('t%d' % when + message)
    # Wait until the last notes have sounded (plus the synths' latency)
    end = start + total_beats * score.beat_ms + alles.ALLES_LATENCY_MS
    time.sleep(max(0, (end - amy.millis()) / 1000.0))


def stop():
    # Sent a few times so a dropped packet can't leave a note hanging
    for i in range(3):
        alles.send(reset=amy.RESET_ALL_NOTES)


def main():
    parser = argparse.ArgumentParser(description='A short piece for two Alles synths.')
    parser.add_argument('--bpm', type=float, default=112)
    parser.add_argument('--volume', type=float, default=None, help='synth volume to set after the reset')
    args = parser.parse_args()

    clients = alles.sync()
    print('Found %d synth(s): %s' % (len(clients), ', '.join('client %d (.%d)' % (c, v['ipv4']) for c, v in sorted(clients.items()))))
    if len(clients) < 2:
        print('This piece is written for two synths; with one, both parts play on it.')

    setup(args.volume)
    score, sections, total_beats = compose(args.bpm)
    print('Playing %d events, %.0f seconds' % (len(score.events), total_beats * score.beat_ms / 1000.0))
    try:
        play(score, sections, total_beats)
    except KeyboardInterrupt:
        pass
    finally:
        stop()


if __name__ == '__main__':
    main()
