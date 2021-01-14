<?php

namespace DevShop\Component\GitRemoteMonitor;

use PHPUnit\Framework\TestCase;

final class DaemonTest extends TestCase
{
    public function testGetInstance(): void
    {
        Daemon::setFilename(__FILE__);
        $this->assertInstanceOf(
            Daemon::class,
            Daemon::getInstance(),
            'Daemon instance failed: no filename.'
        );
    }
}
