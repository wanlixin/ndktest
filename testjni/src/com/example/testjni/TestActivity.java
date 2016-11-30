package com.example.testjni;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Arrays;
import java.util.Comparator;

import android.app.ListActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class TestActivity extends ListActivity {

	@Target(ElementType.METHOD)
	@Retention(RetentionPolicy.RUNTIME)
	public @interface TestCase {
		public int order() default 0;

		public boolean async() default false;

		public String description();
	}

	public static final String TAG = "TestActivity";

	private static final String WAITING;
	private static final TestCase TEST_CASES[];
	private static final Method CMD_METHODS[];

	private final String[] mCmdText = new String[TEST_CASES.length];
	private final int[] mCmdCallCount = new int[TEST_CASES.length];

	static {
		Method[] methods = null;
		TestCase[] testCases = null;
		try {
			Class<?> CasesClasses[] = { MainActivity.class };
			int count = 0;
			for (Class<?> casesClass : CasesClasses) {
				for (Method method : casesClass.getDeclaredMethods()) {
					if ((method.getModifiers() & Modifier.PUBLIC) == 0) {
						continue;
					}
					if (method.getAnnotation(TestCase.class) != null) {
						count++;
					}
				}
			}

			methods = new Method[count];
			int index = 0;
			for (Class<?> casesClass : CasesClasses) {
				for (Method method : casesClass.getDeclaredMethods()) {
					if ((method.getModifiers() & Modifier.PUBLIC) == 0) {
						continue;
					}
					TestCase testCase = method.getAnnotation(TestCase.class);
					if (testCase != null) {
						methods[index] = method;
						index++;
					}
				}
			}

			Arrays.sort(methods, new Comparator<Method>() {

				@Override
				public int compare(Method object1, Method object2) {
					return object1.getAnnotation(TestCase.class).order()
							- object2.getAnnotation(TestCase.class).order();
				}
			});

			testCases = new TestCase[count];
			for (int i = 0; i < count; i++) {
				testCases[i] = methods[i].getAnnotation(TestCase.class);
			}
		} catch (Throwable e) {
			Log.e(TAG, null, e);
		}

		TEST_CASES = testCases;
		CMD_METHODS = methods;

		WAITING = new StringBuilder().append('.').append('.').append('.')
				.toString();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setListAdapter(new BaseAdapter() {

			@Override
			public View getView(int position, View convertView, ViewGroup parent) {
				TextView textView;
				if (convertView == null) {
					float sp = TestActivity.this.getResources()
							.getDisplayMetrics().scaledDensity;
					int padding5 = (int) (5 * sp);

					textView = new TextView(TestActivity.this);
					textView.setPadding(padding5, padding5, padding5, padding5);
					convertView = textView;
				} else {
					textView = (TextView) convertView;
				}
				updateTextView(textView, position);
				return textView;
			}

			@Override
			public long getItemId(int position) {
				return 0;
			}

			@Override
			public Object getItem(int position) {
				return null;
			}

			@Override
			public int getCount() {
				return TEST_CASES.length;
			}
		});
	}

	private void updateTextView(TextView textView, int position) {
		String text = TEST_CASES[position].description() + ':'
				+ mCmdCallCount[position];
		if (mCmdText[position] != null) {
			text += '\n' + mCmdText[position];
		}
		textView.setText(text);
	}

	private String invokeCase(int position) {
		String result = null;
		try {
			try {
				Object value = CMD_METHODS[position].invoke(this);
				if (value == null) {
					result = "done";
				} else {
					result = value.toString();
				}
			} catch (InvocationTargetException e) {
				throw e.getCause();
			}
		} catch (Throwable e) {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			e.printStackTrace(pw);
			result = sw.toString();
		}
		return result;
	}

	@Override
	protected void onListItemClick(ListView l, View view, final int position,
			long id) {
		String result = null;
		if (TEST_CASES[position].async()) {
			if (mCmdText[position] == WAITING) {
				return;
			}
			new Thread(new Runnable() {

				@Override
				public void run() {
					final String result = invokeCase(position);

					runOnUiThread(new Runnable() {

						@Override
						public void run() {
							mCmdText[position] = result;
							((BaseAdapter) getListAdapter()).notifyDataSetChanged();
						}
					});
				}
			}, "RunCase").start();

			result = WAITING;
		} else {
			result = invokeCase(position);
		}

		mCmdText[position] = result;
		mCmdCallCount[position]++;
		updateTextView(((TextView) view), position);
	}
}
