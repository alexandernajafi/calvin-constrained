package ericsson.com.calvin.calvin_constrained;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.os.Debug;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.google.firebase.iid.FirebaseInstanceId;

public class CCActivity extends PreferenceActivity {

    private final String LOG_TAG = "CCActivity log";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        int pid = android.os.Process.myPid();
        Log.d(LOG_TAG, "Application PID: " + pid);
        // Start profiler
        super.onResume();
        CProfiler.messure = false;
        //CProfiler prof = new CProfiler(this, "cc_actor_button");

        //prof.hprof_dumps = true;

        //Thread th = new Thread(prof);
        //th.start();

        getFragmentManager().beginTransaction().replace(android.R.id.content, new CalvinPreferenceFragment()).commit();
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

        if(sp.getBoolean("autostart", false)) {
            CalvinCommon.startService(sp, this);
        }

        Log.d(LOG_TAG, "FCM token: " + FirebaseInstanceId.getInstance().getToken());
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_webpage:
                Uri uri = Uri.parse("https://github.com/EricssonResearch/calvin-base");
                Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                startActivity(intent);
                return true;
            case R.id.action_favorite:
                showDialog();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private void showDialog() {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(this);
        dialogBuilder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
            }
        });
        dialogBuilder.setTitle("About Calvin");
        dialogBuilder.setMessage(getString(R.string.about));
        dialogBuilder.create().show();
    }
}