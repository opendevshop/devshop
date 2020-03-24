<?php

namespace DevShop\Component\PowerProcess;

use Drupal\Console\Core\Style\DrupalStyle;
use Symfony\Component\Console\Formatter\OutputFormatter;
use Symfony\Component\Console\Helper\Helper;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\BufferedOutput;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Console\Terminal;

class PowerProcessStyle extends SymfonyStyle {

    /**
     * @var BufferedOutput
     */
    protected $bufferedOutput;
    protected $input;
    protected $output;
    protected $lineLength;

    /**
     * Icons
     */
    const ICON_HELP = '♥';
    const ICON_EDIT = '✎';
    const ICON_START = '➤';
    const ICON_FINISH = '🏁';
    const ICON_FAILED = '🔥';
    const ICON_COMMAND = '$';
    const ICON_BULLET = '➤';
    const ICON_FOLDER = '📂';
    const ICON_FILE = '🗎';

    public function __construct(InputInterface $input, OutputInterface $output)
    {
        $this->input = $input;
        $this->output = $output;
        $this->bufferedOutput = new BufferedOutput($output->getVerbosity(), false, clone $output->getFormatter());
        // Windows cmd wraps lines as soon as the terminal width is reached, whether there are following chars or not.
        $width = (new Terminal())->getWidth() ?: self::MAX_LINE_LENGTH;
        $this->lineLength = min($width - (int) (DIRECTORY_SEPARATOR === '\\'), self::MAX_LINE_LENGTH);

        parent::__construct($input, $output);
    }

    /**
     * Use to display a directory $ command.
     *
     * @param $message
     * @param string $directory
     */
    public function commandBlock($message, $directory = '') {
        $this->autoPrependBlock();
        $this->customLite($message, $directory . ' <fg=yellow>' . self::ICON_COMMAND . '</>', '');
    }

    public function customLite($message, $prefix = '*', $style = '', $newLine = false)
    {
        if ($style) {
            $message = sprintf(
                '<%s>%s</%s> %s',
                $style,
                $prefix,
                $style,
                $message
            );
        } else {
            $message = sprintf(
                '%s %s',
                $prefix,
                $message
            );
        }
        $this->text($message);
        if ($newLine) {
            $this->newLine();
        }
    }

    /**
     * Output a standard "terminal" looking output.
     * @param $message
     * @param bool $padding
     * @param bool $newline
     */
    public function outputBlock($message, $padding = TRUE, $newline = TRUE) {
        $this->block(
            $message,
            NULL,
            'fg=yellow;bg=black',
            ' ┃ ',
            $padding,
            $newline
        );
    }

    /**
     * Formats a message as a block of text.
     *
     * @param string|array $messages The message to write in the block
     * @param string|null  $type     The block type (added in [] on first line)
     * @param string|null  $style    The style to apply to the whole block
     * @param string       $prefix   The prefix for the block
     * @param bool         $padding  Whether to add vertical padding
     * @param bool         $escape   Whether to escape the message
     */
    public function block($messages, $type = null, $style = null, $prefix = ' ', $padding = false, $escape = true, $newLine = true)
    {
        $messages = \is_array($messages) ? array_values($messages) : [$messages];

        $this->autoPrependBlock();
        $this->write($this->createBlock($messages, $type, $style, $prefix, $padding, $escape));
//
//        if ($newLine) {
//            $this->newLine();
//        }
    }

    /**
     * Display a block of text in the "Help" style.
     * @param $message
     * @param string $icon
     */
    function helpBlock($message, $icon = self::ICON_HELP) {
        if (is_array($message)) {
            $message = implode("\n", $message);
        }
        $this->block(
            " {$icon} {$message}",
            NULL,
            'bg=black;fg=cyan',
            '  ',
            TRUE
        );
    }

    /**
     * Display a block of text in the "Help" style.
     * @param $message
     * @param string $icon
     */
    function titleBlock($message) {
        if (is_array($message)) {
            $message = implode("\n", $message);
        }
        $this->block(
            $message,
            NULL,
            'bg=blue;fg=white',
            '  ',
            TRUE
        );
    }

    /**
     * Replacement for parent::autoPrependBlock(), allowing access and setting newLine to 1 - instead of 2 -.
     */
    private function autoPrependBlock()
    {
        $chars = substr(str_replace(PHP_EOL, "\n", $this->bufferedOutput->fetch()), -2);

        if (!isset($chars[0])) {
            return $this->newLine(); //empty history, so we should start with a new line.
        }
        //Prepend new line for each non LF chars (This means no blank line was output before)
        $this->newLine(1 - substr_count($chars, "\n"));
    }

    private function createBlock($messages, $type = null, $style = null, $prefix = ' ', $padding = false, $escape = false)
    {
        $indentLength = 0;
        $prefixLength = Helper::strlenWithoutDecoration($this->getFormatter(), $prefix);
        $lines = [];

        if (null !== $type) {
            $type = sprintf('[%s] ', $type);
            $indentLength = \strlen($type);
            $lineIndentation = str_repeat(' ', $indentLength);
        }

        // wrap and add newlines for each element
        foreach ($messages as $key => $message) {
            if ($escape) {
                $message = OutputFormatter::escape($message);
            }

            $lines = array_merge($lines, explode(PHP_EOL, wordwrap($message, $this->lineLength - $prefixLength - $indentLength, PHP_EOL, true)));

            if (\count($messages) > 1 && $key < \count($messages) - 1) {
                $lines[] = '';
            }
        }

        $firstLineIndex = 0;
        if ($padding && $this->isDecorated()) {
            $firstLineIndex = 1;
            array_unshift($lines, '');
            $lines[] = '';
        }

        foreach ($lines as $i => &$line) {
            if (null !== $type) {
                $line = $firstLineIndex === $i ? $type.$line : $lineIndentation.$line;
            }

            $line = $prefix.$line;
            $line .= str_repeat(' ', $this->lineLength - Helper::strlenWithoutDecoration($this->getFormatter(), $line));

            if ($style) {
                $line = sprintf('<%s>%s</>', $style, $line);
            }
        }

        return $lines;
    }

    public function bulletLite($message) {
        return $this->customLite($message, '<fg=blue>' . self::ICON_BULLET . '</>');
    }

    /**
     * Wait for a user to press ENTER. Actually just a askHidden() call.
     * @param string $text
     */
    public function pause($text = 'Press ENTER to continue...') {
        $this->askHidden($text, function () {return TRUE;});
    }

    /**
     * {@inheritdoc}
     * @TODO: Remove? Not sure why this is not working in other projects.
     */
    public function isDebug()
    {
        return $this->output->isDebug();
    }
}