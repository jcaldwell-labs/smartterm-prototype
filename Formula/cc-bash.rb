# Homebrew formula for cc-bash
# Claude Code-style bash wrapper with colored output and modern features

class CcBash < Formula
  desc "Claude Code-style bash wrapper with colored output and status bar"
  homepage "https://github.com/jcaldwell-labs/smartterm-prototype"
  url "https://github.com/jcaldwell-labs/smartterm-prototype/archive/refs/tags/v1.2.0.tar.gz"
  # Note: Update sha256 after v1.2.0 release is created
  sha256 "PLACEHOLDER_UPDATE_AFTER_RELEASE"
  license "MIT"
  head "https://github.com/jcaldwell-labs/smartterm-prototype.git", branch: "master"

  depends_on "make" => :build

  def install
    system "make", "CC=#{ENV.cc}"
    bin.install "cc-bash"

    # Install sample config
    (share/"cc-bash").install "cc-bashrc.template"
  end

  def post_install
    # Create user config directory
    (var/"cc-bash").mkpath
  end

  def caveats
    <<~EOS
      To get started with cc-bash:

      1. Copy the sample config to your home directory:
         cp #{share}/cc-bash/cc-bashrc.template ~/.cc-bashrc

      2. Create the plugins directory:
         mkdir -p ~/.cc-bash/plugins

      3. Run cc-bash:
         cc-bash

      For AI integration, see the Python version at:
         https://github.com/jcaldwell-labs/smartterm-prototype#quick-start-python--ai-version
    EOS
  end

  test do
    # Test that binary exists and is executable
    assert_predicate bin/"cc-bash", :exist?
    assert_predicate bin/"cc-bash", :executable?
  end
end
