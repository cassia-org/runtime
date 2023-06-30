package com.cassia

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.Surface
import android.view.SurfaceHolder
import android.widget.TextView
import com.cassia.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity(), SurfaceHolder.Callback {

    private lateinit var binding: ActivityMainBinding
    var serverThread : Thread? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.gameView.holder.addCallback(this)

        serverThread = Thread {
            runServer()
        }
        serverThread?.start()
    }
    override fun surfaceCreated(holder: SurfaceHolder) {
     //   setSurface(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        setSurface(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        TODO("Not yet implemented")
    }

    external fun runServer()

    external fun setSurface(surface: Surface)



    companion object {
        // Used to load the 'cassia' library on application startup.
        init {
            System.loadLibrary("cassia")
        }
    }
}