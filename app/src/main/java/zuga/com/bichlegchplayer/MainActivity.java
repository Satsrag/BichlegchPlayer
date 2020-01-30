package zuga.com.bichlegchplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements BichlegchPlayer.OnPreparedListener {

    private SurfaceView mSurfaceView;
    private final String mPath = "rtmp://58.200.131.2:1935/livetv/hunantv";
    private BichlegchPlayer mBichlegchPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSurfaceView = findViewById(R.id.surface_view);

        mBichlegchPlayer = new BichlegchPlayer();
        mBichlegchPlayer.setSurfaceView(mSurfaceView);
        mBichlegchPlayer.setDataSource(mPath);
        mBichlegchPlayer.setOnPreparedListener(this);
    }

    @Override
    public void onPrepared() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, "prepared success", Toast.LENGTH_SHORT).show();
            }
        });
        mBichlegchPlayer.start();
    }

    @Override
    public void onError(final int errorMode) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, "prepared error: " + errorMode, Toast.LENGTH_SHORT).show();
            }
        });
    }

    public void prepare(View view) {
        mBichlegchPlayer.prepare();
    }
}
