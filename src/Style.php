<?php

namespace jonpugh\ComposerGitBuild;

use Symfony\Component\Console\Style\SymfonyStyle;

class Style extends SymfonyStyle {
    
    /**
     * @param array|string $message
     * @param bool         $newLine
     */
    public function comment($message, $newLine = true)
    {
        $message = sprintf('<comment> %s</comment>', $message);
        if ($newLine) {
            $this->writeln($message);
        } else {
            $this->write($message);
        }
    }
}