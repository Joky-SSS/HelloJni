package com.jokyxray.echo

import android.os.Bundle
import android.os.Handler
import android.os.PersistableBundle
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.ScrollView
import android.widget.TextView
import androidx.annotation.LayoutRes
import androidx.appcompat.app.AppCompatActivity

abstract class AbstractEchoActivity(@LayoutRes val layoutId: Int) : AppCompatActivity(),
    View.OnClickListener {
    private lateinit var startButton: Button
    private lateinit var portEdit: EditText
    private lateinit var logView: TextView
    private lateinit var logScroll: ScrollView
    override fun onCreate(savedInstanceState: Bundle?, persistentState: PersistableBundle?) {
        super.onCreate(savedInstanceState, persistentState)
        setContentView(layoutId)
        startButton = findViewById(R.id.startButton)
        portEdit = findViewById(R.id.portEdit)
        logView = findViewById(R.id.logView)
        logScroll = findViewById(R.id.logScroll)
    }

    override fun onClick(v: View?) {
        if (v == startButton) onStartButtonClicked()
    }

    protected abstract fun onStartButtonClicked()

    protected fun getPort(): Int? {
        return try {
            portEdit.text.toString().toInt()
        } catch (e: Exception) {
            null
        }
    }

    protected fun logMessage(message: String) {
        runOnUiThread {
            logMessageDirect(message)
        }
    }

    protected fun logMessageDirect(message: String) {
        logView.append(message)
        logView.append("\n")
        logScroll.fullScroll(View.FOCUS_DOWN)
    }

    protected abstract inner class AbstractEchoTask : Thread() {
        private val handler = Handler()
        protected open fun onPreExecute() {
            startButton.isEnabled = false
            logView.text = ""
        }

        @Synchronized
        override fun start() {
            onPreExecute()
            super.start()
        }

        override fun run() {
            onBackground();
            handler.post { onPostExecute() }
        }

        protected abstract fun onBackground()

        protected open fun onPostExecute() {
            startButton.isEnabled = true
        }

    }

    companion object {
        init {
            System.loadLibrary("Echo")
        }
    }

}