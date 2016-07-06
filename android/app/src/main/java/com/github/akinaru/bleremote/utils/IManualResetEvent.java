package com.github.akinaru.bleremote.utils;

/**
 * Manual reset event interface
 *
 * @author Bertrand Martel
 */
public interface IManualResetEvent {

    /**
     * Getter for open reset variable
     */
    public boolean getOpen();

    /**
     * Setter for open reset variable
     *
     * @param value used to control monitor state
     */
    public void setOpen(boolean value);

    /**
     * Getter for monitor object
     *
     * @return monitor object used for wait/notify state
     */
    public Object getMonitor();
}
