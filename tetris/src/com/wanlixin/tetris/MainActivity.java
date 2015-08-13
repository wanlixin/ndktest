package com.wanlixin.tetris;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(new TetrisView(this));
	}

	private static class TetrisView extends View {

		private static final int GAME_WIDTH = 10;
		private static final int GAME_HEIGHT = 22;

		private static final Paint[] sPaint;

		private static final Paint sTextPaint;

		private static final int[] sColors = { Color.BLACK, Color.CYAN,
				Color.MAGENTA, Color.LTGRAY, Color.RED, Color.GREEN,
				Color.BLUE, Color.YELLOW };

		static {
			sTextPaint = new Paint();
			sTextPaint.setColor(Color.WHITE);
			sTextPaint.setTextSize(30);

			sPaint = new Paint[sColors.length];
			for (int i = 0; i < sColors.length; i++) {
				Paint paint = new Paint();
				paint.setColor(sColors[i]);
				sPaint[i] = paint;
			}
		}

		private long mLastTick = System.currentTimeMillis();
		private int mFrameCount = 0;
		private int mFps = 0;

		private Rect mRect = new Rect();

		private Game mGame = new Game(GAME_WIDTH, GAME_HEIGHT, 5);

		public TetrisView(Context context) {
			super(context);
		}

		private void DrawBlock(Canvas canvas, int x, int y, int size, int type) {
			mRect.set(x + 1, y + 1, x + size - 1, y + size - 1);
			canvas.drawRect(mRect, sPaint[type]);
		}

		@Override
		protected void onDraw(Canvas canvas) {
			super.onDraw(canvas);

			mGame.tick();

			int size = Math.min(getWidth() / GAME_WIDTH, getHeight()
					/ GAME_HEIGHT);

			for (int i = 0; i < GAME_WIDTH; i++) {
				for (int j = 0; j < GAME_HEIGHT; j++) {
					DrawBlock(canvas, i * size, j * size, size,
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

			canvas.drawText("fps: " + mFps + " cleared: " + mGame.mGameCleared
					+ " died: " + mGame.mGameDied, 0, getHeight() - 100,
					sTextPaint);

			invalidate();
		}
	}
}
