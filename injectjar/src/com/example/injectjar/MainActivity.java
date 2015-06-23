package com.example.injectjar;

import java.io.Closeable;
import java.io.OutputStream;
import java.lang.reflect.Method;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ListActivity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseArray;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class MainActivity extends ListActivity {
	public static final String TAG = "injectjar";

	private static class ProcessInformation {
		protected ProcessInformation(String n, String p, int u, Drawable i) {
			name = n;
			packageName = p;
			pid = u;
			icon = i;
		}

		protected final String name;
		protected final String packageName;
		protected final int pid;
		protected final Drawable icon;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		final SparseArray<ProcessInformation> processes = new SparseArray<ProcessInformation>();

		PackageManager packageManager = getPackageManager();
		for (RunningAppProcessInfo processInfo : ((ActivityManager) getSystemService(ACTIVITY_SERVICE))
				.getRunningAppProcesses()) {

			if (processInfo.pkgList == null || processInfo.pkgList.length == 0) {
				continue;
			}
			try {
				ApplicationInfo info = packageManager.getApplicationInfo(
						processInfo.pkgList[0], 0);

				if ((info.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
					processes.put(
							info.uid,
							new ProcessInformation((String) packageManager
									.getApplicationLabel(info),
									info.packageName, processInfo.pid,
									packageManager.getApplicationIcon(info)));
				}
			} catch (Throwable e) {
				Log.e(TAG, e.getMessage(), e);
			}
		}

		getListView().setOnItemClickListener(new OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {
				run_with_sudo(processes.get(processes.keyAt(position)).pid);
			}
		});

		setListAdapter(new BaseAdapter() {

			@Override
			public View getView(int position, View convertView, ViewGroup parent) {
				try {
					final Context context = MainActivity.this;
					LinearLayout itemRoot;
					LinearLayout itemBanner;
					TextView itemCaption;
					ImageView itemIcon;

					if (convertView == null) {
						float dp = context.getResources().getDisplayMetrics().density;
						float sp = context.getResources().getDisplayMetrics().scaledDensity;
						int padding10 = (int) (10 * sp);

						itemRoot = new LinearLayout(context);
						itemRoot.setPadding(padding10, padding10, padding10,
								padding10);
						itemRoot.setOrientation(LinearLayout.VERTICAL);
						itemBanner = new LinearLayout(context);
						itemIcon = new ImageView(context);
						int icon_size = (int) (48 * dp);
						itemBanner.addView(itemIcon,
								new LinearLayout.LayoutParams(icon_size,
										icon_size));
						itemCaption = new TextView(context);
						itemCaption.setPadding(padding10, 0, 0, 0);
						itemBanner.addView(itemCaption);
						itemRoot.addView(itemBanner);
					} else {
						itemRoot = (LinearLayout) convertView;
						itemBanner = (LinearLayout) itemRoot.getChildAt(0);
						itemIcon = (ImageView) itemBanner.getChildAt(0);
						itemCaption = (TextView) itemBanner.getChildAt(1);
					}

					final ProcessInformation packageInformation = processes
							.get(processes.keyAt(position));
					itemIcon.setImageDrawable(packageInformation.icon);
					itemCaption.setText(packageInformation.name + "\n"
							+ packageInformation.packageName + " (pid:"
							+ packageInformation.pid + ")");
					return itemRoot;
				} catch (Throwable e) {
					Log.e(TAG, e.getMessage(), e);
					return null;
				}
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
				return processes.size();
			}
		});
	}

	protected static void closeCloseable(Closeable closeable) {
		if (closeable == null)
			return;

		try {
			closeable.close();
		} catch (Throwable e) {
		}
	}

	protected static void sudo(String cmd, String[] args, String[] envs)
			throws Throwable {
		OutputStream outputStream = null;
		try {
			Process p = Runtime.getRuntime().exec("su");
			outputStream = p.getOutputStream();
			if (envs != null) {
				for (String env : envs) {
					outputStream.write(("export " + env + '\n').getBytes());
				}
			}
			outputStream.write(cmd.getBytes());
			if (args != null) {
				for (String arg : args) {
					outputStream.write((' ' + arg).getBytes());
				}
			}
			outputStream.write("\nexit\n".getBytes());
			outputStream.flush();
			p.waitFor();
			return;
		} finally {
			closeCloseable(outputStream);
		}
	}

	protected static String findLibrary(String libName) throws Throwable {
		ClassLoader classLoader = MainActivity.class.getClassLoader();
		Method method = classLoader.getClass().getMethod("findLibrary",
				String.class);
		method.setAccessible(true);
		return (String) method.invoke(classLoader, libName);
	}

	protected void run_with_sudo(int pid) {
		try {
			String path = getPackageCodePath();
			sudo("app_process",
					new String[] { "/system/bin", RunInSu.class.getName(),
							Integer.toString(pid), findLibrary("injector"),
							path, AlertView.class.getName() },
					new String[] { "CLASSPATH=" + path });
		} catch (Throwable e) {
			Log.e(TAG, e.getMessage(), e);
		}
	}
}
