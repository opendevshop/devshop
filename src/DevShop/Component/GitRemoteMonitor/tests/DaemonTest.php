<?php

namespace DevShop\Component\GitRemoteMonitor;

use PHPUnit\Framework\TestCase;

final class RemoteMonitorDaemonTest extends TestCase
{
    public function testGetInstance(): void
    {
        RemoteMonitorDaemon::setFilename(__FILE__);
        $this->assertInstanceOf(
            \Core_Daemon::class,
            RemoteMonitorDaemon::getInstance(),
            'Daemon instance failed: no filename.'
        );
    }
}
