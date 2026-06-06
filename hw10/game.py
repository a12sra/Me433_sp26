import pgzrun
import serial
import random

PORT = '/dev/tty.usbmodem1101'   # Mac — change to COM3 etc on Windows
try:
    ser = serial.Serial(PORT, timeout=0.01)
    serial_ok = True
    print('Serial opened:', ser.name)
except Exception as e:
    serial_ok = False
    print('Serial not found:', e, '— keyboard-only mode')

WIDTH  = 480
HEIGHT = 640
TITLE  = 'Flappy Pico'

GRAVITY      = 0.5
FLAP_FORCE   = -9.0
PIPE_SPEED   = 3.5
PIPE_GAP     = 160
PIPE_WIDTH   = 60
PIPE_SPACING = 220
BIRD_X       = 100

SKY    = (113, 197, 207)
GROUND = (222, 184, 135)
GREEN  = (100, 200, 80)
DARK_G = (60,  140, 50)
YELLOW = (255, 220, 50)
WHITE  = (255, 255, 255)
BLACK  = (0,   0,   0)
RED    = (220, 60,  60)
ORANGE = (255, 140, 0)

bird_y = HEIGHT // 2
bird_vel = 0.0
pipes = []
score = 0
best  = 0
alive = True
started = False
frame = 0
prev_flap = 0
prev_restart = 0

def reset():
    global bird_y, bird_vel, pipes, score, alive, started, frame
    bird_y   = HEIGHT // 2
    bird_vel = 0.0
    pipes    = []
    score    = 0
    alive    = True
    started  = False
    frame    = 0
    _spawn_pipe(WIDTH + 50)
    _spawn_pipe(WIDTH + 50 + PIPE_SPACING)

def _spawn_pipe(x):
    gap_y = random.randint(140, HEIGHT - 140 - PIPE_GAP)
    pipes.append({'x': x, 'gap_y': gap_y, 'scored': False})

def read_serial():
    if not serial_ok:
        return 0, 0
    try:
        line = ser.readline()
        if not line:
            return 0, 0
        s = line.decode('utf-8', errors='ignore').strip()
        l = s.find('(')
        r = s.find(')')
        c = s.find(',')
        if l == -1 or r == -1 or c == -1:
            return 0, 0
        return int(s[l+1:c]), int(s[c+1:r])
    except Exception:
        return 0, 0

def update():
    global bird_y, bird_vel, alive, started, score, best, frame
    global prev_flap, prev_restart

    flap_raw, restart_raw = read_serial()
    flap_edge    = (flap_raw    == 1 and prev_flap    == 0)
    restart_edge = (restart_raw == 1 and prev_restart == 0)
    prev_flap    = flap_raw
    prev_restart = restart_raw

    if keyboard.space:
        flap_edge = True
    if keyboard.r:
        restart_edge = True

    if restart_edge and not alive:
        reset()
        return
    if not alive:
        return
    if flap_edge:
        started  = True
        bird_vel = FLAP_FORCE
    if not started:
        return

    frame    += 1
    bird_vel += GRAVITY
    bird_y   += bird_vel

    if bird_y >= HEIGHT - 40 or bird_y <= 0:
        _die(); return

    for p in pipes:
        p['x'] -= PIPE_SPEED

    for p in pipes:
        if not p['scored'] and p['x'] + PIPE_WIDTH < BIRD_X:
            p['scored'] = True
            score += 1
            if score > best:
                best = score

    pipes[:] = [p for p in pipes if p['x'] > -PIPE_WIDTH - 10]
    if pipes and pipes[-1]['x'] < WIDTH - PIPE_SPACING:
        _spawn_pipe(WIDTH + 20)

    bx1, by1, bx2, by2 = BIRD_X-14, bird_y-14, BIRD_X+14, bird_y+14
    for p in pipes:
        px1, px2   = p['x'], p['x'] + PIPE_WIDTH
        top_bottom = p['gap_y']
        bot_top    = p['gap_y'] + PIPE_GAP
        if bx2 > px1 and bx1 < px2:
            if by1 < top_bottom or by2 > bot_top:
                _die(); return

def _die():
    global alive
    alive = False

def draw():
    screen.fill(SKY)
    screen.draw.filled_rect(Rect(0, HEIGHT - 40, WIDTH, 40), GROUND)

    for p in pipes:
        x, gap_y = p['x'], p['gap_y']
        screen.draw.filled_rect(Rect(x, 0, PIPE_WIDTH, gap_y), GREEN)
        screen.draw.filled_rect(Rect(x-4, gap_y-18, PIPE_WIDTH+8, 18), DARK_G)
        bot_top = gap_y + PIPE_GAP
        screen.draw.filled_rect(Rect(x, bot_top, PIPE_WIDTH, HEIGHT-bot_top), GREEN)
        screen.draw.filled_rect(Rect(x-4, bot_top, PIPE_WIDTH+8, 18), DARK_G)

    _draw_bird(BIRD_X, int(bird_y))

    screen.draw.text(str(score), center=(WIDTH//2, 40),
                     fontsize=56, color=WHITE, shadow=(2,2), scolor=BLACK)
    screen.draw.text('Best: '+str(best), topright=(WIDTH-10, 10),
                     fontsize=22, color=WHITE, shadow=(1,1), scolor=BLACK)
    screen.draw.text('GP14=FLAP  GP15=RESTART  (or SPACE/R)',
                     bottomleft=(6, HEIGHT-6), fontsize=16, color=BLACK)

    if not started and alive:
        screen.draw.text('FLAPPY PICO', center=(WIDTH//2, HEIGHT//2-80),
                         fontsize=52, color=YELLOW, shadow=(3,3), scolor=ORANGE)
        screen.draw.text('Press GP14 (or SPACE) to start',
                         center=(WIDTH//2, HEIGHT//2),
                         fontsize=24, color=WHITE, shadow=(1,1), scolor=BLACK)

    if not alive:
        screen.draw.text('GAME OVER', center=(WIDTH//2, HEIGHT//2-60),
                         fontsize=52, color=RED, shadow=(3,3), scolor=BLACK)
        screen.draw.text('Score: '+str(score), center=(WIDTH//2, HEIGHT//2),
                         fontsize=34, color=WHITE, shadow=(2,2), scolor=BLACK)
        screen.draw.text('Press GP15 (or R) to restart',
                         center=(WIDTH//2, HEIGHT//2+50),
                         fontsize=22, color=WHITE, shadow=(1,1), scolor=BLACK)

def _draw_bird(x, y):
    screen.draw.filled_circle((x, y), 18, YELLOW)
    screen.draw.filled_circle((x-10, y+6), 10, ORANGE)
    screen.draw.filled_circle((x+7, y-5), 5, WHITE)
    screen.draw.filled_circle((x+9, y-5), 3, BLACK)
    screen.draw.filled_rect(Rect(x+12, y, 10, 6), ORANGE)

reset()
pgzrun.go()