import pygame
import serial

ser = serial.Serial(
    "/dev/tty.usbmodem1101",
    115200,
    timeout=1
)


pygame.init()

screen = pygame.display.set_mode((800,400))

font = pygame.font.SysFont(None,50)

angle = 0
force = 0

running = True

while running:

    for e in pygame.event.get():
        if e.type == pygame.QUIT:
            running = False

    try:

        line = ser.readline().decode().strip()

        a,f = line.split(",")

        angle = float(a)
        force = int(f)

    except:
        pass

    screen.fill((0,0,0))

    screen.blit(
        font.render(
            f"Angle: {angle:.1f}",
            True,
            (255,255,255)
        ),
        (50,50)
    )

    screen.blit(
        font.render(
            f"Force: {force}",
            True,
            (255,255,255)
        ),
        (50,120)
    )

    if abs(force) > 100000:

        screen.blit(
            font.render(
                "FORCE NOTED",
                True,
                (255,0,0)
            ),
            (50,220)
        )

    pygame.display.flip()

pygame.quit()