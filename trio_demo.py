# trio_demo.py
# A short piece for three Alles synths, placed left, center and right of the listener.
#
#   python3 trio_demo.py               # play the whole piece
#   python3 trio_demo.py --identify    # each synth pings its position (1 = left, 2 = center, 3 = right)
#   python3 trio_demo.py --bpm 96      # slower
#   python3 trio_demo.py --volume 2    # set the synths' volume first (a reset puts it back to 1)
#
# Each synth has its own part of the band, so it only loads the patches it plays. That keeps every
# synth within the 64 oscillators of an Atom Voice (no PSRAM):
#   left   - rhythm:  drums (38 oscs) + bass (6) + rotor (12) = 56
#   center - harmony: pad (18) + electric piano (16) + rotor (12) = 46
#   right  - melody:  marimba (16) + flute (8) + rotor (12) = 36
# The rotor is the same pluck bell on all three, for sounds that move around the room.
#
# Sections:
#   1. Roll call - a D major chord built left, center, right, then each part says hello
#   2. Round     - a three part canon, one synth after another, each in its own sound
#   3. Rotation  - an arpeggio spinning around the room, then turning the other way
#   4. Band      - left: drums + bass, center: pad + keys, right: flute melody
#   5. Relay     - a one bar phrase handed from synth to synth, then all three together
#   6. Ending    - one chord spread across the room, and a last spin that fades out
import argparse
import time
import alles
from alles import amy
from duo_demo import Score, play, stop, DRUMS

LEFT, CENTER, RIGHT, ALL = 0, 1, 2, None
POSITIONS = ['left', 'center', 'right']

ROTOR, BASS, PAD, KEYS, LEAD, FLUTE = 1, 2, 3, 4, 5, 6
SETUP = [
    # client, synth, patch, voices
    (ALL, ROTOR, 52, 2),       # Juno A75 Pluck Bell
    (LEFT, DRUMS, 258, 1),     # General MIDI drum kit from the built in 808 samples
    (LEFT, BASS, 36, 1),       # Juno A55 Synth Bass II
    (CENTER, PAD, 47, 3),      # Juno A68 Synth Pad
    (CENTER, KEYS, 138, 2),    # DX7 E.PIANO 1
    (RIGHT, LEAD, 149, 2),     # DX7 MARIMBA
    (RIGHT, FLUTE, 151, 1),    # DX7 FLUTE 1
]
# The synth each position plays tunes with
TUNE_SYNTH = {LEFT: ROTOR, CENTER: KEYS, RIGHT: LEAD}
KICK, SNARE, CLAP, CLOSED_HAT, OPEN_HAT, COWBELL = 36, 38, 39, 42, 46, 56

D, A, BM, G = (
    {'root': 50, 'pad': [62, 66, 69], 'arp': [69, 74, 78, 81]},
    {'root': 45, 'pad': [61, 64, 69], 'arp': [69, 73, 76, 81]},
    {'root': 47, 'pad': [62, 66, 71], 'arp': [71, 74, 78, 83]},
    {'root': 43, 'pad': [62, 67, 71], 'arp': [67, 71, 74, 79]},
)

# The round: four two bar segments over a repeating | D | A |, so any of them sound together.
# (beat, note, beats)
ROUND = [
    [(0, 74, 2), (2, 78, 2), (4, 76, 2), (6, 73, 2)],
    [(0, 74, .5), (.5, 73, .5), (1, 74, .5), (1.5, 69, .5), (2, 66, 1), (3, 69, 1),
     (4, 73, 1), (5, 76, 1), (6, 69, 1.5), (7.5, 64, .5)],
    [(0, 66, .5), (.5, 69, .5), (1, 74, .5), (1.5, 69, .5), (2, 78, .5), (2.5, 74, .5), (3, 69, .5), (3.5, 74, .5),
     (4, 76, .5), (4.5, 73, .5), (5, 69, .5), (5.5, 73, .5), (6, 76, .5), (6.5, 81, .5), (7, 76, 1)],
    [(0, 62, 1.5), (1.5, 62, .5), (2, 66, 1), (3, 69, 1), (4, 64, 1.5), (5.5, 64, .5), (6, 61, 1), (7, 57, 1)],
]
ROUND_TUNE = [(seg * 8 + b, n, beats) for seg, notes in enumerate(ROUND) for (b, n, beats) in notes]

# The flute's tune in the band section, over D A Bm G twice
FLUTE_TUNE = [(0, 78, 3), (3, 76, .5), (3.5, 74, .5), (4, 76, 2), (6, 73, 2), (8, 74, 3), (11, 71, 1),
              (12, 74, 2), (14, 79, 1), (15, 78, 1), (16, 81, 3), (19, 78, 1), (20, 76, 2), (22, 73, 1),
              (23, 76, 1), (24, 74, 2), (26, 78, 2), (28, 79, 2), (30, 78, 1), (31, 76, 1)]


def drums(s, beat, bars=1, fill=False, light=False):
    for bar in range(bars):
        b = beat + bar * 4
        if light:
            s.hit(b, KICK, 0.7, LEFT)
            s.hit(b + 2, KICK, 0.6, LEFT)
            for k in range(4):
                s.hit(b + k + 0.5, CLOSED_HAT, 0.35, LEFT)
            continue
        for k in (0, 1.5, 2, 2.75):
            s.hit(b + k, KICK, 0.9, LEFT)
        for k in (1, 3):
            s.hit(b + k, SNARE, 0.7, LEFT)
        for k in range(8):
            s.hit(b + k * 0.5, OPEN_HAT if k == 7 else CLOSED_HAT, 0.35 if k % 2 else 0.5, LEFT)
        if fill and bar == bars - 1:
            for k in (3, 3.25, 3.5, 3.75):
                s.hit(b + k, CLAP, 0.5, LEFT)


def bassline(s, beat, prog, bars, walking=True):
    # Kept around D3, the Atoms' small speakers don't reach much lower
    for bar in range(bars):
        root = prog[bar % len(prog)]['root']
        b = beat + bar * 4
        if walking:
            pattern = [(0, root, .75), (1, root + 12, .4), (1.5, root, .4), (2, root, .75),
                       (3, root + 12, .4), (3.5, root + 7, .4)]
        else:
            pattern = [(0, root, 1.8), (2, root, 1.8)]
        for (k, n, beats) in pattern:
            s.note(b + k, BASS, n, beats, vel=0.8, client=LEFT)


def pads(s, beat, prog, bars, vel=0.35):
    for bar in range(bars):
        for n in prog[bar % len(prog)]['pad']:
            s.note(beat + bar * 4, PAD, n, 4, vel=vel, client=CENTER)


def comping(s, beat, prog, bars):
    # Electric piano stabs on the off beats, top two notes of the chord
    for bar in range(bars):
        top = prog[bar % len(prog)]['pad'][1:]
        for k in (0.5, 1.5, 2.5, 3.5):
            for n in top:
                s.note(beat + bar * 4 + k, KEYS, n + 12, 0.4, vel=0.45, client=CENTER)


def spin(s, beat, notes, step, order, vel=0.7, fade=0):
    # Play notes in turn around the room, one synth after another
    for i, n in enumerate(notes):
        s.note(beat + i * step, ROTOR, n, step, vel=max(0.1, vel - fade * i), client=order[i % len(order)])


def compose(bpm):
    s = Score(bpm)
    sections = []
    beat = 0
    around = [LEFT, CENTER, RIGHT]
    back = [RIGHT, CENTER, LEFT]

    # 1. Roll call: D, F# and A come in left to right and hold, then each part plays its own sounds
    sections.append((beat, 'Roll call'))
    for i, (client, n) in enumerate(zip(around, [62, 66, 69])):
        s.note(beat + i, ROTOR, n, 4 - i, vel=0.8, client=client)
    s.hit(beat + 4, KICK, 0.9, LEFT)
    s.hit(beat + 4.5, SNARE, 0.7, LEFT)
    s.note(beat + 4.5, BASS, 50, 1, vel=0.8, client=LEFT)
    for n in (66, 69, 74):
        s.note(beat + 5.5, PAD, n, 1.5, vel=0.5, client=CENTER)
    s.phrase(beat + 6.5, LEAD, [(0, 81, .25), (.25, 78, .25), (.5, 74, .25), (.75, 81, .75)], client=RIGHT)
    spin(s, beat + 8, [74, 78, 81, 86, 81, 78, 74, 69], 0.5, around)
    beat += 12

    # 2. Round: the same eight bar tune, left first, center two bars later, right two bars after that
    sections.append((beat, 'Round'))
    for i, (client, transpose) in enumerate([(LEFT, 0), (CENTER, 0), (RIGHT, 12)]):
        start = beat + i * 8
        s.phrase(start, TUNE_SYNTH[client], ROUND_TUNE, vel=0.75, client=client, transpose=transpose)
        s.phrase(start + 32, TUNE_SYNTH[client], ROUND_TUNE, vel=0.75, client=client, transpose=transpose)
    pads(s, beat + 16, [D, A], bars=16, vel=0.25)
    bassline(s, beat + 16, [D, A], bars=16, walking=False)
    drums(s, beat + 32, bars=12, light=True)
    beat += 80

    # 3. Rotation: eighth notes spin left to right, then back the other way, and speed up to sixteenths
    sections.append((beat, 'Rotation'))
    prog = [BM, G, D, A]
    pattern = [0, 1, 2, 3, 2, 1, 2, 3]
    for bar in range(8):
        arp = prog[bar % 4]['arp']
        if bar < 6:
            spin(s, beat + bar * 4, [arp[i] for i in pattern], 0.5, around if bar < 4 else back)
        else:
            spin(s, beat + bar * 4, [arp[i] for i in pattern * 2], 0.25, back, vel=0.6)
    pads(s, beat, prog, bars=8, vel=0.3)
    bassline(s, beat, prog, bars=8, walking=False)
    for bar in range(4, 8):
        for k in range(4):
            s.hit(beat + bar * 4 + k, KICK, 0.5 + 0.1 * (bar - 4), LEFT)
    s.hit(beat + 31.5, CLAP, 0.6, LEFT)
    beat += 32

    # 4. Band: left plays drums and bass, center pad and keys, right the flute tune
    sections.append((beat, 'Band'))
    prog = [D, A, BM, G]
    drums(s, beat, bars=8, fill=True)
    bassline(s, beat, prog, bars=8)
    pads(s, beat, prog, bars=8, vel=0.3)
    comping(s, beat, prog, bars=8)
    s.phrase(beat, FLUTE, FLUTE_TUNE, vel=0.85, client=RIGHT)
    beat += 32

    # 5. Relay: a one bar phrase goes around the room, then all three play the last two bars together
    sections.append((beat, 'Relay'))
    drums(s, beat, bars=8, fill=True)
    bassline(s, beat, prog, bars=8)
    pads(s, beat, prog, bars=8, vel=0.3)
    for bar in range(8):
        a = prog[bar % 4]['arp']
        lick = [(0, a[0], .5), (.5, a[1], .5), (1, a[2], .5), (1.5, a[3], .5), (2, a[2], .5), (2.5, a[1], .5), (3, a[2], 1)]
        players = around[bar % 3:bar % 3 + 1] if bar < 6 else around
        for client in players:
            s.phrase(beat + bar * 4, TUNE_SYNTH[client], lick, vel=0.75, client=client,
                     transpose=12 if client == RIGHT else 0)
    beat += 32

    # 6. Ending: a D chord low on the left, in the middle at the center, high on the right
    sections.append((beat, 'Ending'))
    s.note(beat, BASS, 50, 6, vel=0.9, client=LEFT)
    s.note(beat, ROTOR, 62, 4, vel=0.7, client=LEFT)
    for n in (66, 69, 74):
        s.note(beat, PAD, n, 7, vel=0.45, client=CENTER)
    s.note(beat, LEAD, 86, 3, vel=0.7, client=RIGHT)
    s.note(beat, FLUTE, 81, 6, vel=0.6, client=RIGHT)
    s.hit(beat, KICK, 1.0, LEFT)
    s.hit(beat, OPEN_HAT, 0.7, LEFT)
    spin(s, beat + 4, [86, 81, 78, 74, 69, 66, 62], 0.5, back, vel=0.6, fade=0.07)
    beat += 10

    return s, sections, beat


def setup(volume=None):
    alles.send(reset=amy.RESET_ALL_OSCS + amy.RESET_SEQUENCER)
    # Wait for the reset to play out (after the synths' latency), or it would wipe the synths we set up next
    time.sleep(alles.ALLES_LATENCY_MS / 1000.0 + 0.3)
    if volume is not None:
        alles.send(volume=volume)
    for (client, synth, patch, voices) in SETUP:
        kwargs = {} if client is None else {'client': client}
        alles.send(synth=synth, patch=patch, num_voices=voices, **kwargs)
        time.sleep(0.1)


def identify(bpm):
    # Synth n pings n + 1 times, so they can be placed left, center and right
    s = Score(bpm)
    for client in (LEFT, CENTER, RIGHT):
        for i in range(client + 1):
            s.note(client * 4 + i * 0.5, ROTOR, 74 + 4 * client, 0.4, client=client)
    return s, [], 12


def main():
    parser = argparse.ArgumentParser(description='A short piece for three Alles synths.')
    parser.add_argument('--bpm', type=float, default=104)
    parser.add_argument('--volume', type=float, default=None, help='synth volume to set after the reset')
    parser.add_argument('--identify', action='store_true', help='ping each synth\'s position instead of playing')
    args = parser.parse_args()

    clients = alles.sync()
    print('Found %d synth(s):' % len(clients))
    for c, v in sorted(clients.items()):
        print('  client %d (.%d): %s' % (c, v['ipv4'], POSITIONS[c] if c < len(POSITIONS) else 'not used'))
    if len(clients) < 3:
        print('This piece is written for three synths; with fewer, parts double up and may not fit on an Atom Voice.')

    setup(args.volume)
    score, sections, total_beats = identify(args.bpm) if args.identify else compose(args.bpm)
    print('Playing %d events, %.0f seconds' % (len(score.events), total_beats * score.beat_ms / 1000.0))
    try:
        play(score, sections, total_beats)
    except KeyboardInterrupt:
        pass
    finally:
        stop()


if __name__ == '__main__':
    main()
