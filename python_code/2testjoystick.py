import pygame

BLACK = (0,0,0)
WHITE = (255,255,255)

# This is a simple class that will help us cprint to the screen
# It has nothing to do with the joysticks, just outputing the
# information.
class TextPrint:
    def __init__(self):
        self.reset()
        self.font = pygame.font.Font(None, 20)

    def cprint(self, screen, textString):
        textBitmap = self.font.render(textString, True, BLACK)
        #screen.blit(textBitmap, [self.x, self.y])
        self.y += self.line_height
        
    def reset(self):
        self.x = 10
        self.y = 10
        self.line_height = 15
        
    def indent(self):
        self.x += 10
        
    def unindent(self):
        self.x -= 10
    
screen = 0

pygame.init()

#Loop until the user clicks the close button.
done = False

# Used to manage how fast the screen updates
clock = pygame.time.Clock()

# Initialize the joysticks
pygame.joystick.init()
    
# Get ready to cprint
textPrint = TextPrint()

# -------- Main Program Loop -----------
while done==False:
    # EVENT PROCESSING STEP
    for event in pygame.event.get(): # User did something
	pass 
    # DRAWING STEP
    # First, clear the screen to white. Don't put other drawing commands

    textPrint.reset()

    # Get count of joysticks
    joystick_count = pygame.joystick.get_count()

   # textPrint.cprint(screen, "Number of joysticks: {}".format(joystick_count) )
   # textPrint.indent()
    
    # For each joystick:
    for i in [0]:
        joystick = pygame.joystick.Joystick(i)
        joystick.init()
    
        
        # Usually axis run in pairs, up/down for one, and left/right for
        # the other.
        axes = joystick.get_numaxes()
        textPrint.cprint(screen, "Number of axes: {}".format(axes) )
        textPrint.indent()
        
        for i in range( axes ):
            axis = joystick.get_axis( i )
            textPrint.cprint(screen, "Axis {} value: {:>6.3f}".format(i, axis) )
        textPrint.unindent()

        # Hat switch. All or nothing for direction, not like joysticks.
        # Value comes back in an array.


    
    # ALL CODE TO DRAW SHOULD GO ABOVE THIS COMMENT
    
    # Go ahead and update the screen with what we've drawn.
    

    # Limit to 20 frames per second
    clock.tick(20)

pygame.quit()
