package com.example.testjni;

import android.util.Log;

public class MainActivity extends TestActivity {

	static {
		try {
			Runtime.getRuntime().loadLibrary("testjni");
		} catch (Throwable e) {
			Log.e(TAG, null, e);
		}
	}

	@TestCase(description = "test")
	public int test1() {
		return test1("hello", "world");
	}

	@TestCase(description = "test2")
	public String test2() {
		return test2("hello", "world");
	}

	protected native int test1(String... args);

	protected native String test2(String... args);
}
