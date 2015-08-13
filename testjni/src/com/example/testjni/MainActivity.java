package com.example.testjni;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.Field;

import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class MainActivity extends ListActivity {

	private static final String CMD_010test1 = "test1";
	private static final String CMD_020test2 = "test2";

	protected static final String CMD_TEXT[];

	protected final String[] mCmdText = new String[CMD_TEXT.length];
	protected final int[] mCmdCallCount = new int[CMD_TEXT.length];

	static {
		String[] cmd = null;
		try {
			int count = 0;
			for (Field field : MainActivity.class.getDeclaredFields()) {
				if (field.getName().startsWith("CMD_")
						&& field.getType() == String.class) {
					count++;
				}
			}
			cmd = new String[count];
			int index = 0;
			for (Field field : MainActivity.class.getDeclaredFields()) {
				if (field.getName().startsWith("CMD_")
						&& field.getType() == String.class) {
					cmd[index] = (String) field.get(null);
					index++;
				}
			}
		} catch (Throwable e) {
		}
		CMD_TEXT = cmd;

		Runtime.getRuntime().loadLibrary("testjni");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setListAdapter(new BaseAdapter() {

			@Override
			public View getView(int position, View convertView, ViewGroup parent) {
				TextView textView;
				if (convertView == null) {
					float sp = MainActivity.this.getResources()
							.getDisplayMetrics().scaledDensity;
					int padding5 = (int) (5 * sp);

					textView = new TextView(MainActivity.this);
					textView.setPadding(padding5, padding5, padding5, padding5);
					convertView = textView;
				} else {
					textView = (TextView) convertView;
				}
				textView.setTag(CMD_TEXT[position]);
				String text = CMD_TEXT[position] + ':'
						+ mCmdCallCount[position];
				if (mCmdText[position] != null) {
					text += '\n' + mCmdText[position];
				}
				textView.setText(text);
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
				return CMD_TEXT.length;
			}
		});
	}

	@Override
	protected void onListItemClick(ListView l, View v, final int position,
			long id) {
		String cmdText = (String) v.getTag();
		String result = null;
		try {
			if (CMD_010test1 == cmdText) {
				result = String.valueOf(test1("hello", "world"));
			} else if (CMD_020test2 == cmdText) {
				result = test2("hello", "world");
			} else {
				result = "not implemented";
			}
			if (result == null) {
				result = "done";
			}
		} catch (Throwable e) {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			e.printStackTrace(pw);
			result = sw.toString();
		}

		mCmdText[position] = result;
		String text = cmdText + ':' + (++mCmdCallCount[position]);
		if (result != null) {
			text += '\n' + result;
		}
		((TextView) v).setText(text);
	}

	protected native int test1(String... args);

	protected native String test2(String... args);
}
