package com.example.threads

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        nativeInit()
        start_button.setOnClickListener {
            val threads = threads_edit.text.toString().toInt()
            val iterations = iterations_edit.text.toString().toInt()
            if (threads > 0 && iterations > 0) {
                startThreads(threads, iterations)
            }
        }
    }

    override fun onDestroy() {
        nativeFree()
        super.onDestroy()

    }

    private fun onNativeMessage(message: String) {
        runOnUiThread {
            log_view.append(message)
            log_view.append("\n")
        }
    }

    private fun javaThreads(threads: Int, iterations: Int) {
        for (i in 0 until threads) {
            val thread = Thread {
                nativeWorker(i, iterations)
            }
            thread.start()
        }
    }

    private external fun posixThreads(threads: Int, iterations: Int)

    private fun startThreads(threads: Int, iterations: Int) {
//        javaThreads(threads, iterations)
        posixThreads(threads,iterations)
    }

    private external fun nativeInit()

    private external fun nativeFree()

    private external fun nativeWorker(id: Int, iterations: Int)

    init {
        System.loadLibrary("Threads")
    }
}
