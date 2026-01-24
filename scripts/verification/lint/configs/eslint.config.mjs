import eslint from '@eslint/js';
import stylistic from '@stylistic/eslint-plugin';
import globals from 'globals';
import { defineConfig } from 'eslint/config';

// ESLint Configuration
export default defineConfig(
    eslint.configs.recommended,
    stylistic.configs.recommended,
    {
        languageOptions: {
            sourceType: 'module',
            globals: {
                ...globals.node
            }
        },
        rules: {
            '@stylistic/quotes': [
                'error',
                'single',
                {
                    allowTemplateLiterals: 'avoidEscape'
                }
            ],
            '@stylistic/indent': [
                'error',
                4,
                {
                    SwitchCase: 1
                }
            ],
            '@stylistic/comma-dangle': [
                'error',
                'never'
            ],
            '@stylistic/quote-props': [
                'error',
                'as-needed'
            ],
            '@stylistic/semi': [
                'error',
                'always'
            ],
            '@stylistic/eol-last': [
                'error',
                'never'
            ],
            '@stylistic/lines-between-class-members': 'off',
            '@stylistic/object-curly-spacing': 'off',
            '@stylistic/brace-style': [
                'error',
                '1tbs'
            ],
            '@stylistic/member-delimiter-style': [
                'error',
                {
                    multiline: {
                        delimiter: 'semi',
                        requireLast: true
                    },
                    singleline: {
                        delimiter: 'comma',
                        requireLast: false
                    }
                }
            ],
            eqeqeq: 'error',
            'no-duplicate-imports': 'error',
            'no-unused-vars': [
                'error',
                {
                    caughtErrors: 'none'
                }
            ]
        }
    }
);