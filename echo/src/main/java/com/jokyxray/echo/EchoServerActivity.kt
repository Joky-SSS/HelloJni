package com.jokyxray.echo

import android.os.Bundle


class EchoServerActivity : AbstractEchoActivity(R.layout.activity_echo_server) {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

    }

    override fun onStartButtonClicked() {
        val port: Int? = getPort()
        port?.let {
            ServerTask(port).start()
        }
    }

    @Throws(Exception::class)
    private external fun nativeStartTcpServer(port: Int)

    @Throws(Exception::class)
    private external fun nativeStartUdpServer(port: Int)

    private inner class ServerTask(val port: Int) : AbstractEchoTask() {
        override fun onBackground() {
            logMessage("Starting Server.")
            try {
                nativeStartTcpServer(port)
            } catch (e: Exception) {
                logMessage(e.message!!)
            }
            logMessage("Server terminated.")
        }
    }
}