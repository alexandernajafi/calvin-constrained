package ericsson.com.calvin.calvin_constrained;

/**
 * Created by alexander on 2017-04-14.
 */

import android.app.ActivityManager;
import android.app.Instrumentation;
import android.content.Context;
import android.os.Debug;
import android.os.Environment;
import android.util.Log;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.LinkedList;
import java.util.List;

/**
 * Created by alexander on 2017-04-14.
 */

public class CProfiler implements Runnable {

	private static final String LOG_TAG = "PROFILER";
	static PrintWriter output;
	boolean running;
	Context context;
	static ActivityManager a;
	static int[] pids = {android.os.Process.myPid()};
	static long time;
	static String filename;
	static boolean hprof_dumps = false;
	static boolean messure = true;

	public CProfiler(Context context, String filename) {
		if(!messure)
			return;

		a = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		try {
			output = new PrintWriter(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/"+filename+".txt", "UTF-8");
			Log.d("PROFILER", "Writing to " + Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/"+filename+".txt");
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		running = true;
		this.context = context;
		this.filename = filename;
	}

	public static synchronized void writeMemInfo(String key) {
		if (!messure)
			return;

		if (hprof_dumps) {
			dumpHprof(key);
		} else {
			Debug.MemoryInfo[] mi = a.getProcessMemoryInfo(pids);
			time = System.currentTimeMillis();
			output.write(time + ":NPD:" + key + ":" + mi[0].nativePrivateDirty + "\n");
			output.write(time + ":OPD:" + key + ":" + mi[0].otherPrivateDirty + "\n");
			output.write(time + ":TPD:" + key + ":" + mi[0].getTotalPrivateDirty() + "\n");
			output.flush();
		}
	}

	public static void dumpHprof(String key) {
		try {
			time = System.currentTimeMillis();
			Debug.dumpHprofData(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/"+time+"_"+filename+"_"+key+".hprof");
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void run() {
		while(true) {
			this.writeMemInfo("none");
			System.gc();
			try {
				Thread.sleep(200);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}