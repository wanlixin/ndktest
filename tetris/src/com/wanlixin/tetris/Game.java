package com.wanlixin.tetris;

import java.util.Random;

public class Game {
	private static final int sBlockData[][][][][] = {
			{ { { { -1, 0 }, { -1, 1 }, { 0, 0 }, { 1, 0 } }, null },
					{ { { -1, 0 }, { 0, 0 }, { 0, 1 }, { 0, 2 } }, null },
					{ { { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 } }, null },
					{ { { -1, 0 }, { -1, 1 }, { -1, 2 }, { 0, 2 } }, null }, },

			{ { { { -1, 0 }, { 0, 0 }, { 1, 0 }, { 1, 1 } }, null },
					{ { { -1, 2 }, { 0, 0 }, { 0, 1 }, { 0, 2 } }, null },
					{ { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } }, null },
					{ { { -1, 0 }, { -1, 1 }, { -1, 2 }, { 0, 0 } }, null }, },

			{ { { { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 } }, null },
					{ { { -1, 1 }, { -1, 2 }, { 0, 0 }, { 0, 1 } }, null }, },

			{ { { { -1, 1 }, { 0, 0 }, { 0, 1 }, { 1, 0 } }, null },
					{ { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 0, 2 } }, null }, },

			{ { { { -1, 0 }, { -1, 1 }, { 0, 0 }, { 0, 1 } }, null }, },

			{ { { { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 0 } }, null },
					{ { { -1, 1 }, { 0, 0 }, { 0, 1 }, { 0, 2 } }, null },
					{ { { -1, 1 }, { 0, 0 }, { 0, 1 }, { 1, 1 } }, null },
					{ { { -1, 0 }, { -1, 1 }, { -1, 2 }, { 0, 1 } }, null }, },

			{ { { { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 } }, null },
					{ { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 } }, null }, } };

	private static final int EMPTY = -1;

	private static final Random sRandom = new Random();

	static {
		int type_count = sBlockData.length;
		for (int type_index = 0; type_index < type_count; type_index++) {
			int[][][][] type = sBlockData[type_index];

			int direction_count = type.length;
			for (int direction_index = 0; direction_index < direction_count; direction_index++) {
				int[][][] direction = type[direction_index];
				int[][] block = direction[0];
				int minX = Integer.MAX_VALUE;
				int maxX = Integer.MIN_VALUE;
				int minY = Integer.MAX_VALUE;
				int maxY = Integer.MIN_VALUE;
				int data_count = block.length;
				for (int data_index = 0; data_index < data_count; data_index++) {
					int[] data = block[data_index];
					minX = Math.min(minX, data[0]);
					maxX = Math.max(maxX, data[0]);
					minY = Math.min(minY, data[1]);
					maxY = Math.max(maxY, data[1]);
				}
				int[] tail = new int[maxX - minX + 1];
				for (int i = 0; i < tail.length; i++) {
					tail[i] = Integer.MIN_VALUE;
				}
				for (int data_index = 0; data_index < data_count; data_index++) {
					int[] data = block[data_index];
					int x = data[0] - minX;
					tail[x] = Math.max(tail[x], data[1]);
				}
				direction[1] = new int[][] { { minX, minY }, { maxX, maxY },
						tail };
			}
		}
	}

	public final int mWidth;
	public final int mHeight;
	public final int mDifficulty;

	private int mCurrentBlockIndex;
	private int mCurrentBlockDirection;
	private int mCurrentBlockX;
	private int mCurrentBlockY;

	public int[][] mFrameData;

	private int mBestDirection;
	private int mBestX;

	public int mGameDied;
	public int mGameCleared;

	public Game(int width, int height, int difficulty) {
		mWidth = width;
		mHeight = height;
		mDifficulty = difficulty;
		mFrameData = new int[mHeight][mWidth];

		mGameDied = 0;
		mGameCleared = 0;

		newGame();
	}

	private void newGame() {
		for (int i = 0; i < mWidth; i++) {
			for (int j = 0; j < mHeight; j++) {
				if (j < mHeight - mDifficulty) {
					mFrameData[j][i] = EMPTY;
				} else {
					mFrameData[j][i] = sRandom.nextInt(sBlockData.length + 1) - 1;
				}
			}
		}

		newBlock();
	}

	private void newBlock() {
		clear();

		mCurrentBlockIndex = sRandom.nextInt(sBlockData.length);
		mCurrentBlockDirection = sRandom
				.nextInt(sBlockData[mCurrentBlockIndex].length);

		int[][][] direction = sBlockData[mCurrentBlockIndex][mCurrentBlockDirection];
		int[][] block = direction[0];
		int[] min = direction[1][0];
		int[] max = direction[1][1];

		mCurrentBlockX = (mWidth - max[0] + min[0] - 1) / 2 - min[0];
		mCurrentBlockY = 0 - min[1];

		int data_count = block.length;
		for (int data_index = 0; data_index < data_count; data_index++) {
			int[] data = block[data_index];
			mFrameData[mCurrentBlockY + data[1]][mCurrentBlockX + data[0]] = mCurrentBlockIndex;
		}
	}

	private void clear() {
		boolean[] clear = new boolean[mHeight];
		for (int y = 0; y < mHeight; y++) {
			clear[y] = true;
			for (int x = 0; x < mWidth; x++) {
				if (mFrameData[y][x] == EMPTY) {
					clear[y] = false;
					break;
				}
			}

		}
		int y2 = mHeight - 1;
		for (int y1 = mHeight - 1; y1 >= 0; y1--) {
			boolean found = false;
			for (; y2 >= 0; y2--) {
				if (!clear[y2]) {
					found = true;
					break;
				} else {
					mGameCleared++;
				}
			}
			if (found) {
				if (y1 != y2) {
					for (int x = 0; x < mWidth; x++) {
						mFrameData[y1][x] = mFrameData[y2][x];
					}
				}
				y2--;
			} else {
				for (int x = 0; x < mWidth; x++) {
					mFrameData[y1][x] = EMPTY;
				}
			}
		}
	}

	private boolean step(int x, int y, int direction) {
		int[][][][] type = sBlockData[mCurrentBlockIndex];
		while (direction < 0) {
			direction += type.length;
		}
		while (direction >= type.length) {
			direction -= type.length;
		}
		int[][][] directionOld = type[mCurrentBlockDirection];
		int[][] blockOld = directionOld[0];
		int[] minOld = directionOld[1][0];
		int[][][] directionNew = type[direction];
		int[][] blockNew = directionNew[0];
		int[] minNew = directionNew[1][0];
		int[] maxNew = directionNew[1][1];

		if (x + minNew[0] < 0 || (x + maxNew[0] >= mWidth)) {
			return false;
		}
		if (y + maxNew[1] >= mHeight) {
			newBlock();
			return false;
		}
		int dataOld_count = blockOld.length;
		for (int dataOld_index = 0; dataOld_index < dataOld_count; dataOld_index++) {
			int[] dataOld = blockOld[dataOld_index];
			mFrameData[mCurrentBlockY + dataOld[1]][mCurrentBlockX + dataOld[0]] = EMPTY;
		}
		int dataNew_count = blockNew.length;
		for (int dataNew_index = 0; dataNew_index < dataNew_count; dataNew_index++) {
			int[] dataNew = blockNew[dataNew_index];
			if (mFrameData[y + dataNew[1]][x + dataNew[0]] != EMPTY) {
				for (int dataOld_index = 0; dataOld_index < dataOld_count; dataOld_index++) {
					int[] dataOld = blockOld[dataOld_index];
					mFrameData[mCurrentBlockY + dataOld[1]][mCurrentBlockX
							+ dataOld[0]] = mCurrentBlockIndex;
				}
				if (y != mCurrentBlockY) {
					if ((mCurrentBlockY + minOld[1]) == 0) {
						mGameDied++;
						newGame();
					} else {
						newBlock();
					}
					return true;
				}
				return false;
			}
		}

		mCurrentBlockX = x;
		mCurrentBlockY = y;
		mCurrentBlockDirection = direction;
		int[][] block = sBlockData[mCurrentBlockIndex][mCurrentBlockDirection][0];
		int data_count = block.length;
		for (int data_index = 0; data_index < data_count; data_index++) {
			int[] data = block[data_index];
			mFrameData[mCurrentBlockY + data[1]][mCurrentBlockX + data[0]] = mCurrentBlockIndex;
		}
		return true;
	}

	private void rotate(boolean left) {
		step(mCurrentBlockX, mCurrentBlockY, mCurrentBlockDirection
				+ (left ? -1 : 1));
	}

	private void move(boolean left) {
		step(mCurrentBlockX + (left ? -1 : 1), mCurrentBlockY,
				mCurrentBlockDirection);
	}

	private void moveDown() {
		step(mCurrentBlockX, mCurrentBlockY + 1, mCurrentBlockDirection);
	}

	public void tick() {
		int[][][][] type = sBlockData[mCurrentBlockIndex];
		int[][][] directionOld = type[mCurrentBlockDirection];
		int[][] blockOld = directionOld[0];

		int dataOld_count = blockOld.length;
		for (int dataOld_index = 0; dataOld_index < dataOld_count; dataOld_index++) {
			int[] dataOld = blockOld[dataOld_index];
			mFrameData[mCurrentBlockY + dataOld[1]][mCurrentBlockX + dataOld[0]] = EMPTY;
		}
		int bestScore = Integer.MIN_VALUE;
		mBestDirection = Integer.MAX_VALUE;
		mBestX = Integer.MAX_VALUE;
		for (int directionIndex = 0; directionIndex < type.length; directionIndex++) {
			int[][][] direction = type[directionIndex];
			int[][] block = direction[0];
			int[] min = direction[1][0];
			int[] max = direction[1][1];
			int[] tail = direction[1][2];
			int x1 = -min[0];
			int x2 = mWidth - max[0];
			for (int x = x1; x < x2; x++) {
				int y;
				for (y = mCurrentBlockY; y < mHeight; y++) {
					boolean conflict = false;
					int data_count = block.length;
					for (int data_index = 0; data_index < data_count; data_index++) {
						int[] data = block[data_index];
						if ((y + data[1] >= mHeight)
								|| (mFrameData[y + data[1]][x + data[0]] != EMPTY)) {
							conflict = true;
							break;
						}
					}
					if (conflict) {
						y--;
						break;
					}
				}

				if (y < mCurrentBlockY) {
					continue;
				}

				int score = 0;
				int holeCount = 0;
				for (int i = 0; i < tail.length; i++) {
					int testX = x + min[0] + i;
					for (int j = y + tail[i] + 1; j < mHeight; j++) {
						if (mFrameData[j][testX] == EMPTY) {
							holeCount++;
						}
						break;
					}
				}

				score -= holeCount * 2;
				score += y;

				if (score > bestScore) {
					bestScore = score;
					mBestDirection = directionIndex;
					mBestX = x;
				}
			}
		}
		for (int dataOld_index = 0; dataOld_index < dataOld_count; dataOld_index++) {
			int[] dataOld = blockOld[dataOld_index];
			mFrameData[mCurrentBlockY + dataOld[1]][mCurrentBlockX + dataOld[0]] = mCurrentBlockIndex;
		}

		int x = mCurrentBlockX;
		int y = mCurrentBlockY;
		int direction = mCurrentBlockDirection;

		if (mBestDirection == mCurrentBlockDirection
				&& mBestX == mCurrentBlockX) {
			y++;
		} else {
			if (mBestDirection != mCurrentBlockDirection) {
				direction += 1;
			}
			if (mBestX < mCurrentBlockX) {
				x--;
			} else if (mBestX > mCurrentBlockX) {
				x++;
			}
		}
		if (!step(x, y, direction)) {
			moveDown();
		}
	}
}
