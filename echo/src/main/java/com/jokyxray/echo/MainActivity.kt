package com.jokyxray.echo

import android.os.Bundle

class MainActivity : AbstractEchoActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }
}
