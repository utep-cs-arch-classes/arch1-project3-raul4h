/*
Author: Raul Hinostroza
ID: 80532365
Instructor: Eric Freudenthal
TA: Daniel Cervantes
Last Date of Modification: 11/29/2016 at 10:30 PM
*/
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

char scoreRed[6] = "Red: 0";
char scoreBlue[10] = "Blue: 0   ";

static short duration = 15;
static short note = 0;
int end = 0;

AbRect rect11 = {abRectGetBounds, abRectCheck, {4,14}};		//Left Paddle
AbRect rect10 = {abRectGetBounds, abRectCheck, {4,14}}; 	//Right paddle

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2, screenHeight/2 - 10}
};

Layer ball = {		/**< Layer with an white circle */
  (AbShape *)&circle5,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  0,
};


Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  &ball
};

Layer rightBar = {		/**< Layer with right paddle */
  (AbShape *)&rect10,
  {screenWidth-10, screenHeight/2}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLUE,
  &fieldLayer,
};

Layer leftBar = {		/**< Layer with left paddle */
  (AbShape *)&rect11,
  {10, screenHeight/2}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &rightBar,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml3 = { &ball, {1,2}, 0 }; /**< not all layers move */
MovLayer ml2 = { &leftBar, {0,0}, &ml3 };	//Left Paddle
MovLayer ml1 = { &rightBar, {0,0}, &ml2 };	//Right Paddle

movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ){
	  int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	  newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);		//Init with 4 buttons

  shapeInit();
  buzzer_init();

  layerInit(&leftBar);
  layerDraw(&leftBar);

  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      readSwRight(&ml2);	//Read right bar button input
      readSwLeft(&ml1);		//Read left bar button input
      drawString5x7(0,0,scoreRed,COLOR_WHITE,COLOR_BLACK);	//Draw and update red score
      drawString5x7(screenWidth/2+10,screenHeight-10,scoreBlue,COLOR_WHITE,COLOR_BLACK);	//Draw and update blue score
      checkScore();				//Update score
      checkCollisionBall();			//Bounce ball of paddles
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml1, &leftBar);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count =0;
  static short count2 = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count++;
  count2++;
  if (count == 15) {
    mlAdvance(&ml1, &fieldFence);
    if(p2sw_read())
      redrawScreen = 1;
    count = 0;
  }
  if(count2 == duration){		//Play note when ball hits paddle
    buzzer_set_period(note);
    count2 = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}

int readSwRight(MovLayer* ml){
  u_int switches = p2sw_read();

  if(!(switches & (1<<0))){		//If siwtch 1 is pressed, move paddle up
     ml->velocity.axes[1] = -5;
    }
    else if(!(switches & (1<<1)))	//If switch 2 is pressed, move paddle down
      ml->velocity.axes[1] = 5;
    else				//Else don't move
      ml->velocity.axes[1] = 0;
}
  
int readSwLeft(MovLayer* ml){
  u_int switches = p2sw_read();

    if(!(switches & (1<<2)))		//If switch 3 is pressed, move paddle up
      ml->velocity.axes[1] = -5;
    else if(!(switches & (1<<3)))	//If switch 4 is pressed, move paddle down
      ml->velocity.axes[1] = 5;
    else				//Else don't move
      ml->velocity.axes[1] = 0;
}
int checkScore(){
  if(ball.pos.axes[0] > screenWidth){	//If position of ball is outside of width increase red score
    increaseScoreRed(scoreRed);
    if(scoreRed[5] == '5'){		//Red Wins
      ml3.velocity.axes[0] = 0;
      ml3.velocity.axes[1] = 0;
      drawString5x7(screenWidth/2-30,screenHeight/2-40,"RED WINS!",COLOR_WHITE,COLOR_BLACK);
      end = 1;
    }
    else{				//Reset ball
      ml3.velocity.axes[0] = 1;
      ml3.velocity.axes[1] = 2;
    }
    ball.posNext.axes[0] = screenWidth/2;
    ball.posNext.axes[1] = screenHeight/2;
  }
  else if(ball.pos.axes[0] < 0){	//If ball is less than 0 in width, increase blue
    increaseScoreBlue(scoreBlue);
    if(scoreBlue[6] == '5'){		//Blue wins
      ml3.velocity.axes[0] = 0;
      ml3.velocity.axes[1] = 0;
      drawString5x7(screenWidth/2-30,screenHeight/2-40,"BLUE WINS!",COLOR_WHITE,COLOR_BLACK);
      end = 1;
    }
    else{				//Reset ball
      ml3.velocity.axes[0] = -1;
      ml3.velocity.axes[1] = -2;
    }
    ball.posNext.axes[0] = screenWidth/2;
    ball.posNext.axes[1] = screenHeight/2;
  }
}
int checkCollisionBall(){
  if(withinPaddle()){			
    static char count = 0;
    buzzer_set_period(1000);		//Make sound
    ml3.velocity.axes[0] = -ml3.velocity.axes[0];	//Invert direction in x-axis
    if(ml3.velocity.axes[0] > 0){			//Everytime ball hits paddle, increase speed on x-axis to increase difficulty
      ml3.velocity.axes[0]++;
      count++;
    }
    else{
	ml3.velocity.axes[0]--;
	count++;
    }
    if(count == 2){					//For every two increases in x-axis, increase 1 in y-axis
      if(ml3.velocity.axes[1] > 0){
	ml3.velocity.axes[1]++;
	count++;
      }
      else{
	ml3.velocity.axes[1]--;
	count++;
      }
    }
  }
}
//Use abRectCheck to see if ball hits paddle
int withinPaddle(){
  Vec2 ballRight = {ball.pos.axes[0]+circle5.radius,ball.pos.axes[1]};		//Ball with right paddle
  Vec2 ballLeft = {ball.pos.axes[0]-circle5.radius,ball.pos.axes[1]};		//Ball with left paddle

  if(abRectCheck(&rect10,&rightBar.pos,&ballRight))
    return 1;
  else if(abRectCheck(&rect11,&leftBar.pos,&ballLeft))
    return 1;
  return 0;
}
