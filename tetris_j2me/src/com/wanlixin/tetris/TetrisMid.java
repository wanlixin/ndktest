package com.wanlixin.tetris;

import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.game.GameCanvas;
import javax.microedition.midlet.MIDlet;
import javax.microedition.midlet.MIDletStateChangeException;

public class TetrisMid extends MIDlet {
	protected void startApp() {
		Display.getDisplay(this).setCurrent(new TetrisCanvas());
	}

	protected void destroyApp(boolean arg0) throws MIDletStateChangeException {
	}

	protected void pauseApp() {
	}

	public static class TetrisCanvas extends GameCanvas {

		private static final int GAME_WIDTH = 10;
		private static final int GAME_HEIGHT = 22;

		private static final int[] sColors = { 0xFFFFFFFF, 0xFF00FFFF,
				0xFFFF00FF, 0xFFCCCCCC, 0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
				0xFFFFFF00 };

		private long mLastTick = System.currentTimeMillis();
		private int mFrameCount = 0;
		private int mFps = 0;

		private Game mGame = new Game(GAME_WIDTH, GAME_HEIGHT, 5);

		public TetrisCanvas() {
			super(false);
		}

		private void DrawBlock(Graphics g, int x, int y, int size, int type) {
			g.setColor(sColors[type]);
			g.fillRect(x, y, size - 1, size - 1);
		}

		public void paint(Graphics g) {
			super.paint(g);

			mGame.tick();

			int size = Math.min(getWidth() / GAME_WIDTH, getHeight()
					/ GAME_HEIGHT);

			for (int i = 0; i < GAME_WIDTH; i++) {
				for (int j = 0; j < GAME_HEIGHT; j++) {
					DrawBlock(g, i * size, j * size, size,
							mGame.mFrameData[j][i] + 1);
				}
			}
			mFrameCount++;
			long tick = System.currentTimeMillis();
			if ((tick - mLastTick) > 1000) {
				mFps = (int) ((mFrameCount * 1000) / (tick - mLastTick));
				mFrameCount = 0;
				mLastTick = tick;
			}

			g.setColor(0xffFF00FF);
			g.drawString("fps: " + mFps + " cleared: " + mGame.mGameCleared
					+ " died: " + mGame.mGameDied, 0, getHeight() - 20, 0);

			repaint();
		}
	}
}
