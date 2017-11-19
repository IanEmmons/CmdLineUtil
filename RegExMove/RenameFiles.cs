
using System;
using System.IO;
using System.Text.RegularExpressions;

namespace IansUtilities
{
	public class RenameFiles
	{
		private bool			m_isCaseInsensitive;
		private bool			m_suppressOverwritePrompt;
		private bool			m_isVerbose;
		private bool			m_isRecursive;
		private string			m_path;
		private string			m_regExStr;
		private string			m_replacement;

		private DirectoryInfo	m_searchDir;
		private FileInfo		m_file;
		private Regex			m_regex;

		public static void Main(string[] args)
		{
			try
			{
				RenameFiles app = new RenameFiles(args);
				app.run();
			}
			catch (Exception e)
			{
				Console.WriteLine();
				Console.WriteLine("Encountered error:  " + e.ToString());
			}
		}

		private RenameFiles(string[] args)
		{
			m_isCaseInsensitive			= true;
			m_suppressOverwritePrompt	= false;
			m_isVerbose					= false;
			m_isRecursive				= false;
			m_path						= null;
			m_regExStr					= null;
			m_replacement				= null;

			m_searchDir					= null;
			m_file						= null;
			m_regex						= null;

			InterpretCmdLineArgs(args);
			ValidateArgs();
		}

		private void InterpretCmdLineArgs(string[] args)
		{
			foreach (string arg in args)
			{
				if ((arg[0] == '/' || arg[0] == '-') && arg.Length == 2)
				{
					switch (arg.ToLower()[1])
					{
					case 'c':
						m_isCaseInsensitive = false;
						break;

					case 'y':
						m_suppressOverwritePrompt = true;
						break;

					case 'v':
						m_isVerbose = true;
						break;

					case 'r':
						m_isRecursive = true;
						break;

					default:
						Usage("Invalid argument:  " + arg);
						break;
					}
				}
				else if (m_path == null)
				{
					m_path = arg;
				}
				else if (m_regExStr == null)
				{
					m_regExStr = arg;
				}
				else if (m_replacement == null)
				{
					m_replacement = arg;
				}
				else
				{
					Usage("Unexpected or invalid argument:  " + arg);
				}
			}
		}

		private void ValidateArgs()
		{
			if (m_path == null || m_regExStr == null || m_replacement == null)
			{
				Usage("Missing argument.");
			}

			if (Directory.Exists(m_path))
			{
				m_searchDir = new DirectoryInfo(m_path);
			}
			else if (File.Exists(m_path))
			{
				if (m_isRecursive)
				{
					Usage("The /r switch is not permitted when renaming a single file.");
				}
				m_file = new FileInfo(m_path);
			}
			else
			{
				Usage("The path '" + m_path + "' doesn't exist.");
			}

			if (!m_regExStr.StartsWith("^"))
			{
				m_regExStr = "^" + m_regExStr;
			}
			if (!m_regExStr.EndsWith("$"))
			{
				m_regExStr += "$";
			}

			try
			{
				RegexOptions opt = RegexOptions.Singleline;
				if (m_isCaseInsensitive)
				{
					opt |= RegexOptions.IgnoreCase;
				}
				m_regex = new Regex(m_regExStr, opt);
			}
			catch (ArgumentException e)
			{
				Usage("Regular expression parsing error:  " + e.Message);
			}
		}

		private static void Usage(string message)
		{
			if (message != null && message.Length > 0)
			{
				Console.WriteLine();
				Console.WriteLine(message);
			}
			Console.WriteLine();
			Console.WriteLine("    Usage 1:  RenameFiles [ /c ] [ /y ] [ /v ] [ /r ] <dir> <regex> <replacement>");
			Console.WriteLine();
			Console.WriteLine("    Usage 2:  RenameFiles [ /c ] [ /y ] [ /v ] <file> <regex> <replacement>");
			Console.WriteLine();
			Console.WriteLine("    where");
			Console.WriteLine();
			Console.WriteLine("        /c Specifies a case-sensitive search.");
			Console.WriteLine();
			Console.WriteLine("        /y (Potentially Dangerous) Causes renamed files to overwrite existing");
			Console.WriteLine("            files of the same name without prompting.");
			Console.WriteLine();
			Console.WriteLine("        /v Specifies verbose output.");
			Console.WriteLine();
			Console.WriteLine("        /r Specifies a recursive search for matching files.");
			Console.WriteLine();
			Console.WriteLine("        <dir> The directory to search for files to rename.");
			Console.WriteLine();
			Console.WriteLine("        <file> The file to rename.");
			Console.WriteLine();
			Console.WriteLine("        <regex> The regular expression that selects the files whose name is");
			Console.WriteLine("            to be changed.");
			Console.WriteLine();
			Console.WriteLine("        <replacement> The substitution expression that gives the files their");
			Console.WriteLine("            new names.");

			System.Environment.Exit(-1);
		}

		private void run()
		{
			if (m_file != null)
			{
				RenameFile(m_file);
			}
			else if (m_isRecursive)
			{
				EnumerateFilesRecursively(m_searchDir);
			}
			else
			{
				EnumerateFiles(m_searchDir);
			}
		}

		private void EnumerateFilesRecursively(DirectoryInfo searchDir)
		{
			EnumerateFiles(searchDir);

			DirectoryInfo[] subDirs = searchDir.GetDirectories();
			foreach (DirectoryInfo di in subDirs)
			{
				EnumerateFilesRecursively(di);
			}
		}

		private void EnumerateFiles(DirectoryInfo searchDir)
		{
			FileInfo[] files = searchDir.GetFiles();
			foreach (FileInfo file in files)
			{
				RenameFile(file);
			}
		}

		private void RenameFile(FileInfo fileToRename)
		{
			if (m_regex.IsMatch(fileToRename.Name))
			{
				string newFileName = m_regex.Replace(fileToRename.Name, m_replacement);
				if (newFileName != fileToRename.Name)
				{
					string newPathName = Path.Combine(fileToRename.DirectoryName, newFileName);
					if (!File.Exists(newPathName))
					{
						RenameFile(fileToRename, newPathName, newFileName);
					}
					else if (OverwritePermissionGiven(fileToRename, newFileName))
					{
						File.Delete(newPathName);
						RenameFile(fileToRename, newPathName, newFileName);
					}
				}
			}
		}

		private bool OverwritePermissionGiven(FileInfo fileToRename, string newFileName)
		{
			if (m_suppressOverwritePrompt)
			{
				return true;
			}
			else
			{
				Console.Write("    Renaming the file '" + fileToRename.FullName
					+ "' will overwrite '" + newFileName + "'.  Overwrite?  [Y/N] ");
				return Console.In.ReadLine().ToLower() == "y";
			}
		}

		private void RenameFile(FileInfo fileToRename, string newPathName, string newFileName)
		{
			if (m_isVerbose)
			{
				Console.WriteLine("Renaming " + fileToRename.FullName + " to " + newFileName);
			}

			fileToRename.MoveTo(newPathName);
		}
	}
}
