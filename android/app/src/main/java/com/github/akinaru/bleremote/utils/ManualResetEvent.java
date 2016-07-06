package com.github.akinaru.bleremote.utils;

/**
 * <b>Notifies one or more waiting threads that an event has occurred (otherwise
 * timing waiting)</b>
 * <p>
 * <p/>
 * href="http://stackoverflow.com/questions/1064596/what-is-javas-equivalent-of-manualresetevent"
 * </p>
 */
public class ManualResetEvent implements IManualResetEvent {

    /**
     * monitor is an object : it permits to synchronized both WaitOne methods
     * that are using both the variable open.
     */
    private Object monitor = new Object();

    /**
     * variable used to keep or release the wait timer
     */
    private volatile boolean open = false;

    /**
     * set timer parameter false put timer in wait mode if waitOne(X) functions
     * are called
     */
    public ManualResetEvent(boolean open) {
        this.open = open;
    }

    public void removeMonitor() {
        monitor = null;
    }

    /**
     * wait until open is false.
     */
    public boolean waitOne() throws InterruptedException {
        synchronized (monitor) {
            while (open == false) {
                monitor.wait();
            }
        }
        return true;
    }

    /**
     * wait for the required time. And return true only if time has elapsed
     */
    public boolean waitOne(long milliseconds) throws InterruptedException {
        synchronized (monitor) {
            if (open)
                return true;
            monitor.wait(milliseconds);
            return open;
        }
    }

    /**
     * notify monitor object to get up and resume action
     */
    public void set() {
        synchronized (monitor) {
            open = true;
            monitor.notifyAll();
        }
    }

    /**
     * reset is putting the variableopen back to its default value. As a
     * consequence, timer is reinitialized
     */
    public void reset() {
        open = false;
    }

    /**
     * Getter for open reset variable
     *
     * @return open value
     */
    @Override
    public boolean getOpen() {
        return this.open;
    }

    /**
     * Setter for open reset variable
     *
     * @param value
     */
    @Override
    public void setOpen(boolean value) {
        this.open = value;
    }

    /**
     * Getter for monitor object
     *
     * @return monitor object
     */
    @Override
    public Object getMonitor() {
        return this.monitor;
    }
}
