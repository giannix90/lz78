package it.cnr.isti.doremi.sleeplogger;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.content.Intent;
import android.content.IntentSender;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Environment;
import android.support.v13.app.FragmentStatePagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.widget.Toast;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.GoogleApiClient.ConnectionCallbacks;
import com.google.android.gms.common.api.GoogleApiClient.OnConnectionFailedListener;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.wearable.ChannelApi;
import com.google.android.gms.wearable.MessageApi;
import com.google.android.gms.wearable.MessageEvent;
import com.google.android.gms.wearable.Wearable;

import java.io.File;
import java.util.Locale;

/**
 * This is the main activity of the applications: as every activity out
 * there it manages Fragments display through {@link ViewPager} and serves
 * as a mean of communication for them. Also, acts as a middle end between
 * UI and the Service, and manages preferences.
 */

public class MainActivity extends Activity implements
        MainFragment.EventsListener,
        SamplingIntervalFragment.EventsListener,
        UIRefreshFragment.EventsListener, ConnectionCallbacks, OnConnectionFailedListener, BluetoothFragment.GoogleService{

    private static final String TAG = MainActivity.class.getName();
    private static final int REQUEST_RESOLVE_ERROR = 1001;
    private GoogleApiClient mGoogleApiClient;
    private MessageApi.MessageListener messageListener;
    private static final String LOG_PATH = Environment.getExternalStorageDirectory()
            + File.separator + "logs";
    private File dir;



    /**
     * A {@link FragmentStatePagerAdapter} that returns a fragment corresponding to
     * one of the sections/tabs/pages.
     * Since this is a wear application, to avoid too high memory load
     * {@link android.support.v13.app.FragmentStatePagerAdapter} is used.
     */
    public class SectionsPagerAdapter extends FragmentStatePagerAdapter {

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        // getItem is called to instantiate the fragment for the given page.
        @Override
        public Fragment getItem(int position) {
            switch (position) {
                case 0:
                    return new MainFragment();

                case 1:
                    return new SamplingIntervalFragment();

                case 2:
                    return new UIRefreshFragment();

                case 3:
                    return new DeleteLogFileFragment();

                case 4:
                    return new BluetoothFragment();

                default:
                    return null;
            }
        }

        // Returns total number of pages
        @Override
        public int getCount() { return 5; }

        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch (position) {
                case 0:
                    return getString(R.string.title_fragment_main).toUpperCase(l);

                case 1:
                    return getString(R.string.title_fragment_settings).toUpperCase(l);

                case 2:
                    return getString(R.string.title_fragment_settings).toUpperCase(l);

                case 3:
                    return null;//getString(R.string.title_fragment_settings).toUpperCase(l);

                case 4:
                    return null;

                default:
                    return null;
            }
        }
    }

    SectionsPagerAdapter mSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    ViewPager mViewPager;

    private SharedPreferences settings = null;
    private int UI_REFRESH_INTERVAL;

    public boolean isSampling() {
        return SensorSamplingService.isStarted();
    }

    public boolean toggleSampling() {
        Intent samplingIntent = new Intent(MainActivity.this,
                SensorSamplingService.class);
        if (SensorSamplingService.isStarted()) {
            stopService(samplingIntent);
            return false;
        }
        else {
            startService(samplingIntent);
            return true;
        }
    }

    public int getUiRefreshInterval() {
        return UI_REFRESH_INTERVAL;
    }

    public void setUiRefreshInterval(int uiRefreshInterval) {
        if (UI_REFRESH_INTERVAL != uiRefreshInterval)
            UI_REFRESH_INTERVAL = uiRefreshInterval;
    }

    public String getInfo(String format) {
        return String.format(format, SensorSamplingService.getSamples(),
                SensorSamplingService.getRealSamplingFrequency());
    }

    public int getSamplingInterval() {
        return SensorSamplingService.getSamplingInterval();
    }

    public void setSamplingInterval(int samplingInterval) {
        SensorSamplingService.setSamplingInterval(samplingInterval);
    }

    //funzione che restituisce un riferimento al client per googleApiClient al fragment Bluetooth
    public GoogleApiClient get(){
        if (mGoogleApiClient==null)Log.d(TAG,"GoogleapiClient e' NULL!!!!!!!!!!");
        else Log.d(TAG,"GoogleapiClient non è NULL!!!!!!!!!!");
        return mGoogleApiClient;
    }


    public void OpenChannelComm(String nodeId)
    {

        Log.d(TAG,"Sono qui!!");
        ChannelApi.OpenChannelResult result = Wearable.ChannelApi.openChannel(mGoogleApiClient, nodeId, "/log").await();
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        // Create the adapter that will return a fragment for each page of the activity.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);




    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart()");
        super.onStart();

        // Settings must be read here: android doesn't allow so in onCreate()
        if (settings == null) {
            settings = getSharedPreferences("settings.xml", MODE_PRIVATE);
            Resources data = getResources();

            // Second argument is default value, returned if key is not found
            UI_REFRESH_INTERVAL = settings.getInt("UIRefreshInterval",
                    data.getInteger(R.integer.default_ui_refresh_interval));
            SensorSamplingService.setSamplingInterval(settings.getInt("SamplingInterval",
                    data.getInteger(R.integer.default_sampling_interval)));

        }

        //Creo l'oggetto client per connettermi a google service
        mGoogleApiClient = new GoogleApiClient.Builder(this)
                //aggiungo le API che mi servono

                .addApi(Wearable.API)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                //compongo il client con le API
                .build();
        //mi connetto con il client a Google service
        mGoogleApiClient.connect();

        if(mGoogleApiClient.isConnected())
            Log.d(TAG,"--------------Google Service connesso-----------");
    }

    @Override
    protected void onStop()	{
        Log.d(TAG, "onStop()");
        super.onStop();


        Wearable.MessageApi.removeListener(mGoogleApiClient, messageListener);
        mGoogleApiClient.disconnect();

        settings.edit()
                .putInt("SamplingInterval", SensorSamplingService.getSamplingInterval())
                .putInt("UIRefreshInterval", UI_REFRESH_INTERVAL)
                .apply();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause()");
        super.onPause();

    }

    @Override
    protected void onResume() {
        Log.d (TAG, "onResume()");
        super.onResume();
    }

    @Override
    public void onConnected(Bundle connectionHint) {
        Log.d(TAG, "onConnected: " + connectionHint);
        Wearable.MessageApi.addListener(mGoogleApiClient, new MessageApi.MessageListener() {
            @Override
            public void onMessageReceived(MessageEvent messageEvent) {

                //Gestisco l'ACK di conferma ricezione log dallo smartphone
                Log.d(TAG, "Message received: " + messageEvent.getPath());
                runOnUiThread(new Runnable() {
                                  @Override
                                  public void run() {
                                      Toast.makeText(getApplication(), "Log Received!", Toast.LENGTH_SHORT).show();
                                  }
                              });
                        dir = new File(LOG_PATH);
                        if (dir.isDirectory()) {
                            File DelLog = new File(dir, messageEvent.getPath());

                            DelLog.delete();
                            Log.d(TAG, messageEvent.getPath() + " rimosso");
                        }
                    }
                })
                        .setResultCallback(new ResultCallback<Status>() {
                            @Override
                            public void onResult(Status status) {
                                Log.d("Activity", " Listener message aggiunto " + status.getStatusMessage());

                            }
                        });
                // Now you can use the Data Layer API
                //Connected to google play services
            }

            @Override
            public void onConnectionSuspended(int cause) {
                Log.d(TAG, "onConnectionSuspended: " + cause);
            }

            @Override
            public void onConnectionFailed(ConnectionResult result) {
                Log.d(TAG, "onConnectionFailed: " + result);

                //Procedura automatizzata che risolve l'errore, il caso più comune d'errore è statusCode=SIGN_IN_FAILED ovvero problemi di permessi
                boolean mResolvingError = false;
                if (mResolvingError) {
                    // Already attempting to resolve an error.
                    return;
                } else if (result.hasResolution()) {
                    try {
                        mResolvingError = true;
                        result.startResolutionForResult(this, REQUEST_RESOLVE_ERROR);
                    } catch (IntentSender.SendIntentException e) {
                        // There was an error with the resolution intent. Try again.
                        mGoogleApiClient.connect();
                    }
                } else {
                    // Show dialog using GooglePlayServicesUtil.getErrorDialog()
                    Log.d(TAG, String.valueOf(result.getErrorCode()));
                    mResolvingError = true;
                }
            }


        }
