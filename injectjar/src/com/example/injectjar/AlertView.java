package com.example.injectjar;

import android.app.ActivityThread;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class AlertView implements Runnable, OnClickListener {

	private final int mPid;
	private final int mUid;
	private final String mText;
	private final Context mContext;

	private LinearLayout mRootView;
	private WindowManager mWindowManager;

	protected AlertView(int pid, int uid, String text, Context context) {
		mPid = pid;
		mUid = uid;
		mText = text;
		mContext = context;

		new Handler(Looper.getMainLooper()).post(this);
	}

	@Override
	public void run() {
		try {
			float dp = mContext.getResources().getDisplayMetrics().density;
			float sp = mContext.getResources().getDisplayMetrics().scaledDensity;
			int padding20 = (int) (20 * sp);
			int padding10 = (int) (10 * sp);
			int padding5 = (int) (5 * sp);

			PackageManager packageManager = mContext.getPackageManager();
			String packageName = null;
			ApplicationInfo applicationInfo = null;
			try {
				packageName = packageManager.getNameForUid(mUid);
				applicationInfo = packageManager.getApplicationInfo(
						packageName, 0);
				packageName = (String) packageManager
						.getApplicationLabel(applicationInfo);
			} catch (NameNotFoundException e) {
			}
			if (TextUtils.isEmpty(packageName)) {
				packageName = "未知程序";
			}
			packageName += "\nuid:" + mUid + " pid:" + mPid;

			mWindowManager = (WindowManager) mContext
					.getSystemService(Context.WINDOW_SERVICE);

			mRootView = new LinearLayout(mContext);
			mRootView.setOrientation(LinearLayout.VERTICAL);
			mRootView.setBackgroundColor(0xffF5F5F5);

			TextView textView = new TextView(mContext);
			textView.setText("injectjar提示");
			textView.setBackgroundColor(0xff3f4145);
			textView.setTextSize(18);
			textView.setTextColor(0xFFFFFFFF);
			textView.setPadding(padding20, padding10, padding20, padding10);
			textView.setGravity(Gravity.CENTER);
			mRootView.addView(textView);

			LinearLayout linearLayout2 = new LinearLayout(mContext);
			linearLayout2.setOrientation(LinearLayout.VERTICAL);

			textView = new TextView(mContext);
			textView.setText(packageName);
			textView.setTextColor(0xff4d4d4d);
			linearLayout2.setPadding(padding10, 0, 0, 0);
			linearLayout2.addView(textView);

			LinearLayout linearLayout = new LinearLayout(mContext);

			if (applicationInfo != null) {
				ImageView imageView = new ImageView(mContext);
				imageView.setImageDrawable(packageManager
						.getApplicationIcon(applicationInfo));
				int icon_size = (int) (48 * dp);
				linearLayout.addView(imageView, new LinearLayout.LayoutParams(
						icon_size, icon_size));
			}
			linearLayout.addView(linearLayout2);
			linearLayout.setPadding(padding20, padding20, padding20, padding10);

			mRootView.addView(linearLayout);

			if (mText != null) {
				textView = new TextView(mContext);
				textView.setTextColor(0xffF79D1C);
				textView.setText(mText);
				textView.setPadding(padding20, 0, padding20, 0);
				mRootView.addView(textView);
			}

			Button button = new Button(mContext);
			button.setText("关闭");
			button.setOnClickListener(this);
			button.setPadding(padding5, padding10, padding5, padding5);
			mRootView.addView(button);

			LayoutParams layoutParams = new LayoutParams();
			layoutParams.width = LayoutParams.WRAP_CONTENT;
			layoutParams.height = LayoutParams.WRAP_CONTENT;
			layoutParams.type = LayoutParams.TYPE_SYSTEM_ALERT;
			layoutParams.gravity = Gravity.CENTER;
			layoutParams.format = PixelFormat.OPAQUE;
			layoutParams.dimAmount = 0.4f;
			layoutParams.flags = LayoutParams.FLAG_LAYOUT_IN_SCREEN
					| LayoutParams.FLAG_DIM_BEHIND;

			mWindowManager.addView(mRootView, layoutParams);
		} catch (Throwable e) {
			Toast.makeText(mContext, e.getMessage(), Toast.LENGTH_LONG).show();
		}
	}

	@Override
	public void onClick(View v) {
		mWindowManager.removeView(mRootView);
	}

	public static void main() {
		Context context = null;
		try {
			context = ActivityThread.currentApplication();

			if (context != null) {
				new AlertView(android.os.Process.myPid(),
						android.os.Process.myUid(), "来自于injectjar", context);
			}
		} catch (Throwable e) {
			if (context != null) {
				Toast.makeText(context, e.getMessage(), Toast.LENGTH_LONG)
						.show();
			}
		}
	}
}
